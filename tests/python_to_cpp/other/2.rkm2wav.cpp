#define INCLUDE_FS
#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

Array<String> argv;

auto WAV_HEADER_SIZE = 44;
auto SAMPLE_RATE = 44100;
auto BAUD_RATE = 700;
auto SAMPLES_PER_BIT = to_int(SAMPLE_RATE / BAUD_RATE);

auto temp_arr = create_array({255 - 32}) * (SAMPLES_PER_BIT - idiv(SAMPLES_PER_BIT, 2)) + create_array({32}) * (idiv(SAMPLES_PER_BIT, 2));

auto BIT_ZERO_WAVE = copy(temp_arr);

struct CodeBlock1
{
    CodeBlock1()
    {
        temp_arr.reverse();
    }
} code_block_1;

auto BIT_ONE_WAVE = temp_arr;

auto FIR_WEIGHTS = create_array({0.000577926080672738, 0.000459465018502213, 0.000314862245890514, 0.000114058758552773, -0.000176232019410298, -0.000586368131836170, -0.001136970252974637, -0.001832200078313594, -0.002653914944386688, -0.003557457996743626, -0.004469734119871817, -0.005290040439872776, -0.005893881011277897, -0.006139718992073919, -0.005878331252147731, -0.004964157085603880, -0.003267801176075040, -0.000688684837013550, 0.002833242904185398, 0.007307810544085602, 0.012687912264589127, 0.018866545092452867, 0.025678084518976477, 0.032903957573255177, 0.040282515704921813, 0.047522557733791673, 0.054319636536848272, 0.060374030484608231, 0.065409095148934615, 0.069188648369429409, 0.071532089395465556, 0.072326107924878466, 0.071532089395465556, 0.069188648369429409, 0.065409095148934615, 0.060374030484608238, 0.054319636536848279, 0.047522557733791680, 0.040282515704921813, 0.032903957573255184, 0.025678084518976480, 0.018866545092452870, 0.012687912264589122, 0.007307810544085603, 0.002833242904185400, -0.000688684837013550, -0.003267801176075043, -0.004964157085603879, -0.005878331252147732, -0.006139718992073923, -0.005893881011277897, -0.005290040439872781, -0.004469734119871819, -0.003557457996743629, -0.002653914944386688, -0.001832200078313594, -0.001136970252974637, -0.000586368131836170, -0.000176232019410298, 0.000114058758552773, 0.000314862245890515, 0.000459465018502213, 0.000577926080672738});
auto WINDOW_SIZE = FIR_WEIGHTS.len();
auto fir_window = range_el(0, WINDOW_SIZE).map([](const auto &i){return 128;});

template <typename T1> auto fir_low_pass_filter(const T1 &arr)
{
    auto filtered = create_array({Byte(0)}) * arr.len();
    for (auto i : range_el(0, arr.len())) {
        ::fir_window = create_array({arr[i]}) + ::fir_window[range_el(0, ::WINDOW_SIZE - 1)];
        filtered.set(i, to_int(sum_map(range_el(0, ::WINDOW_SIZE), [](const auto &j){return ::fir_window[j] * ::FIR_WEIGHTS[j];})));
    }
    return filtered;
}

template <typename T2> auto wav_write_byte(const File &f, const T2 &b)
{
    for (auto i : range_ee(0, 7)) {
        auto data = (b >> (7 - i)) & 1 ? ::BIT_ONE_WAVE : ::BIT_ZERO_WAVE;
        f.write_bytes(fir_low_pass_filter(data));
    }
    return ::SAMPLES_PER_BIT * 8;
}

auto _debug = false;
template <typename T1> auto debug(const T1 &msg)
{
    if (::_debug)
        print(u"DEBUG: "_S & msg);
}

int MAIN_WITH_ARGV()
{
    INIT_ARGV();

    if (::argv.len() < 2 || in(u"-h"_S, ::argv) || in(u"--help"_S, ::argv)) {
        print(u"usage: 2.rkm2wav INPUT_FILE"_S);
        exit(1);
    }

    auto fin = File(_get<1>(::argv));
    auto fout = FileWr(_get<0>(fs::path::split_ext(_get<1>(::argv))) & u".wav"_S);
    auto addr_start = int_from_bytes_be(fin.read_bytes(2));
    auto addr_end = int_from_bytes_be(fin.read_bytes(2));
    auto data_length = addr_end - addr_start + 1;
    fin.seek(4 + data_length);
    auto checksum = int_from_bytes_be(fin.read_bytes(2));
    fin.seek(4);

    debug(u"start address: 0x#04"_S.format(hex(addr_start)));
    debug(u"end address: 0x#04"_S.format(hex(addr_end)));
    debug(u"checksum: 0x#04"_S.format(hex(checksum)));

    fout.write_bytes(create_array({Byte(0)}) * WAV_HEADER_SIZE);
    auto file_size = WAV_HEADER_SIZE;

    for (auto i : range_ee(0, 255))
        file_size += wav_write_byte(fout, 0x00);

    file_size += wav_write_byte(fout, 0xE6);

    for (auto i : range_ee(0, 1))
        file_size += wav_write_byte(fout, (addr_start >> (1 - i) * 8) & 0xFF);

    for (auto i : range_ee(0, 1))
        file_size += wav_write_byte(fout, (addr_end >> (1 - i) * 8) & 0xFF);

    for (auto i : range_el(0, data_length)) {
        auto b = int_from_bytes_be(fin.read_bytes(1));
        file_size += wav_write_byte(fout, b);
    }

    for (auto i : range_ee(0, 1))
        file_size += wav_write_byte(fout, (checksum >> (1 - i) * 8) & 0xFF);

    fout.write_bytes(fir_low_pass_filter(range_el(0, WINDOW_SIZE).map([](const auto &i){return 128;})));
    file_size += WINDOW_SIZE;

    fout.seek(0);
    fout.write_bytes("RIFF"_B);
    auto temp = file_size - 8;
    fout.write_bytes(bytes_from_int(to_uint32(temp)));
    fout.write_bytes("WAVEfmt "_B);
    temp = 16;
    fout.write_bytes(bytes_from_int(to_uint32(temp)));
    temp = 1;
    fout.write_bytes(bytes_from_int(to_uint16(temp)));
    temp = 1;
    fout.write_bytes(bytes_from_int(to_uint16(temp)));
    temp = SAMPLE_RATE;
    fout.write_bytes(bytes_from_int(to_uint32(temp)));
    temp = SAMPLE_RATE * 1;
    fout.write_bytes(bytes_from_int(to_uint32(temp)));
    temp = 1;
    fout.write_bytes(bytes_from_int(to_uint16(temp)));
    temp = 8;
    fout.write_bytes(bytes_from_int(to_uint16(temp)));
    fout.write_bytes("data"_B);
    temp = file_size - WAV_HEADER_SIZE;
    fout.write_bytes(bytes_from_int(to_uint32(temp)));

    debug(u"done"_S);
}
