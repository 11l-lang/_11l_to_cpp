#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

Array<String> argv;

int MAIN_WITH_ARGV()
{
    INIT_ARGV();

    auto file_name = _get<1>(::argv);
    file_name = _get<0>(file_name.split(u"."_S));

    auto bmp = File(file_name & u".bmp"_S);

    bmp.seek(10, 0);
    auto offset = to_int(int_t_from_bytes<UInt32>(bmp.read_bytes_at_most(4)));

    bmp.seek(18, 0);
    auto bmp_w = to_int(int_t_from_bytes<UInt32>(bmp.read_bytes_at_most(4)));
    auto bmp_h = to_int(int_t_from_bytes<UInt32>(bmp.read_bytes_at_most(4)));

    print(bmp_h & u" "_S & bmp_w);

    bmp.seek(34, 0);
    auto bmp_s = to_int(int_t_from_bytes<UInt32>(bmp.read_bytes_at_most(4)));

    auto bmp_b = to_int(bmp_s / bmp_h);
    print(bmp_h & u" "_S & bmp_w & u" "_S & bmp_s & u" "_S & bmp_b);
    bmp.seek(offset, 0);

    auto bmp_line = u""_S;
    Array<String> bmp_list;
    Array<String> bmp_list_v;

    for (auto line : range_el(0, bmp_h)) {
        for (auto byte : range_el(0, bmp_b)) {
            auto bmp_byte = bmp.read_bytes_at_most(1);
            bmp_line &= bin(255 - _get<0>(bmp_byte)).zfill(8);
        }
        bmp_list.append(bmp_line[range_el(0, bmp_w)]);
        bmp_list_v.append(bmp_line[range_el(0, bmp_w)].replace(u"0"_S, u" "_S));
        bmp_line = u""_S;
    }
    bmp_list_v.reverse();
    for (auto &&line : bmp_list_v)
        print(line);

    auto byte_word = u""_S;
    Array<String> n5110_line;
    Array<Array<String>> n5110_array;

    for (auto line : range_el(0, bmp_h).step(8)) {
        for (auto bit_num : range_el(0, bmp_w)) {
            for (auto bit : range_el(line, line + 8))
                if (bit > bmp_h - 1)
                    byte_word &= u"0"_S;
                else
                    byte_word &= bmp_list[bit][bit_num];
            n5110_line.append(u"0x"_S & hex(to_int(byte_word, 2)).lowercase());
            byte_word = u""_S;
        }
        n5110_array.append(n5110_line);
        n5110_line.drop();
    }

    n5110_array.reverse();

    auto text_file = FileWr(file_name & u".txt"_S);
    text_file.write(u"static unsigned short "_S & file_name & u"_rows = "_S & String(n5110_array.len()) & u";\n"_S);
    text_file.write(u"static unsigned short "_S & file_name & u"_cols = "_S & String(_get<0>(n5110_array).len()) & u";\n"_S);
    text_file.write(u"static unsigned char "_S & file_name & u"[] =\n"_S);
    text_file.write(u"{\n"_S);
    {int Lindex = 0;
    for (auto &&lines : n5110_array) {
        auto l_cnt = Lindex;
        {int Lindex = 0;
        for (auto &&hexa : lines) {
            auto cnt = Lindex;
            text_file.write(hexa);
            if (cnt < lines.len() - 1)
                text_file.write(u","_S);
            Lindex++;
        }}
        if (l_cnt < n5110_array.len() - 1)
            text_file.write(u",\n"_S);
        else
            text_file.write(u"\n"_S);
        Lindex++;
    }}
    text_file.write(u"};"_S);
}
