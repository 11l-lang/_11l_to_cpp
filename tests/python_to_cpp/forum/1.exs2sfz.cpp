#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

Array<String> argv;

struct CodeBlock1
{
    CodeBlock1()
    {
        uR"( EXS24 to SFZ sample library metadata converter.

    Might work. Might awaken some forgotten trickster goddess and lure her to your computer. Enjoy.

    Copyright (c) 2013, vonred

    Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is
    hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
    INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
    FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
    LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. )"_S;
    }
} code_block_1;

Array<Byte> instrument_data;

template <typename T1, typename T2> auto chunk_size(const T1 &instrument_data, const T2 &offset)
{
    return 84 + unpack_from_bytes<UInt32>(instrument_data, offset + 4);
}

class EXSChunk
{
public:

    int offset;
    decltype(0) __size = 0;

    auto size()
    {
        uR"( size is specified in bytes, at byte 4-8, and does not include common chunk elements;
			that is, does not include the first 84 bytes )"_S;

        if (__size == 0)
            __size = chunk_size(::instrument_data, offset);

        return __size;
    }

    auto id()
    {
        uR"( return the chunk's id number; n.b. that this does not seem to actually be used --
			the sequence of chunks of each type in the file is used instead )"_S;

        return unpack_from_bytes<UInt32>(::instrument_data, offset + 8);
    }

    auto name()
    {
        uR"( the name of a chunk starts at byte 20, and has a max. length of 64 bytes;
			this is treated as a zero-terminated utf-8 string )"_S;

        return _get<0>(::instrument_data[range_el(offset + 20, offset + 84)].decode(u"utf-8"_S).split(u"\x00"_S));
    }

    auto display()
    {
        u"\" display the raw chunk data in hexdump format "_S;
        throw NotImplementedError();
    }
};

struct CodeBlock2
{
    CodeBlock2()
    {
        uR"(
		for line in range(0, self.size(), 16):
			print("0x{0:04X}: ".format(line), end='')
			for i in range(0, 16, 4):
				for j in range(0, 4):
					index = line + i + j
					if index >= self.size():
						print("  ", end='')
					else:
						print("{0:02X}".format(instrument_data[self.offset + index]), end='')
				print(" ", end='')
			print(" ", end='')
			end = line + 16
			if end > self.size():
				end = self.size()
			for c in instrument_data[self.offset + line:self.offset + end]:
				if c > 13 and c < 128:
					print(chr(c), end='')
				else:
					print(" ", end='')
			print(""))"_S;
    }
} code_block_2;

auto EXSHeader_sig = 0x0000'0101;

class EXSHeader : public EXSChunk
{
public:

    template <typename T1> EXSHeader(const T1 &offset)
    {
        this->offset = offset;

        if (!(offset == 0))
            throw RuntimeError(u"Found header at location  other than beginning of file! offset is #."_S.format(offset));
    }
};

class EXSZone : public EXSChunk
{
public:

    template <typename T1> EXSZone(const T1 &offset)
    {
        this->offset = offset;
    }

    auto rootnote() const
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 85);
    }

    auto finetune()
    {
        return unpack_from_bytes<Int8>(::instrument_data, offset + 86);
    }

    auto pan()
    {
        return unpack_from_bytes<Int8>(::instrument_data, offset + 87);
    }

    auto volumeadjust()
    {
        return unpack_from_bytes<Int8>(::instrument_data, offset + 88);
    }

    auto startnote() const
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 90);
    }
    auto endnote() const
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 91);
    }

    auto minvel()
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 93);
    }
    auto maxvel()
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 94);
    }

    auto samplestart()
    {
        return unpack_from_bytes<Int32>(::instrument_data, offset + 96);
    }
    auto sampleend()
    {
        return unpack_from_bytes<Int32>(::instrument_data, offset + 100);
    }

    auto loopstart()
    {
        return unpack_from_bytes<Int32>(::instrument_data, offset + 104);
    }
    auto loopend()
    {
        return unpack_from_bytes<Int32>(::instrument_data, offset + 108);
    }

    auto _loop_()
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 117);
    }

    auto pitchtrack() const
    {
        return !(unpack_from_bytes<Byte>(::instrument_data, offset + 84) & 1);
    }
    auto oneshot()
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 84) & 2;
    }

    auto group() const
    {
        auto group = unpack_from_bytes<Int32>(::instrument_data, offset + 172);
        if (group >= 0)
            return group;

        return 0;
    }

    auto sampleindex()
    {
        return unpack_from_bytes<UInt32>(::instrument_data, offset + 176);
    }
};

class EXSGroup : public EXSChunk
{
public:

    template <typename T1> EXSGroup(const T1 &offset)
    {
        this->offset = offset;
    }

    auto polyphony()
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 86);
    }

    auto trigger()
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 157);
    }

    auto output()
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 158);
    }
    auto sequence()
    {
        return unpack_from_bytes<Int32>(::instrument_data, offset + 164);
    }
};

class EXSSample : public EXSChunk
{
public:

    template <typename T1> EXSSample(const T1 &offset)
    {
        this->offset = offset;
    }

    auto length()
    {
        return unpack_from_bytes<Int32>(::instrument_data, offset + 88);
    }

    auto rate()
    {
        return unpack_from_bytes<Int32>(::instrument_data, offset + 92);
    }

    auto bitdepth()
    {
        return unpack_from_bytes<Byte>(::instrument_data, offset + 96);
    }
};

class EXSParam : public EXSChunk
{
public:

    template <typename T1> EXSParam(const T1 &offset)
    {
        this->offset = offset;
    }
};

class EXSSamplePool
{
public:

    Array<String> locations;
    String base;

    EXSSamplePool()
    {
    }

    template <typename T1, typename T2 = decltype(1)> auto locate(T1 filename, const T2 &search_depth = 1)
    {
        uR"( try to locate the directory containing a sample; if found,
			it will be added to the list of known locations which are searched first )"_S;

        auto last = u""_S;
        std::function<String(String)> search_location = [&filename, &last, &search_location, this](const String &search)
        {
            if (in(search, locations))
                return u""_S;

            auto path = fs::path::canonical(fs::path::absolute(fs::path::join(base, search)));
            if (path == last)
                return u""_S;

            for (auto &&name : fs::list_dir(path)) {
                auto location = fs::path::join(path, name);
                if (fs::is_file(location)) {
                    if (name.lowercase() == filename) {
                        locations.append(search);
                        return search;
                    }
                }
                else if (fs::is_dir(location)) {
                    location = search_location(fs::path::join(search, name));
                    if (location != u"")
                        return location;
                }
            }
            return u""_S;
        };

        filename = filename.lowercase();

        for (auto &&location : locations)
            for (auto &&name : fs::list_dir(fs::path::join(base, location)))
                if (name.lowercase() == filename) {
                    if (fs::is_file(fs::path::join(base, location, filename)))
                        return location;
                }

        auto search = u""_S;
        for (auto _i : range_el(0, search_depth)) {
            auto location = search_location(search);
            if (location != u"")
                return location;
            last = fs::path::canonical(fs::path::absolute(fs::path::join(base, search)));
            search = fs::path::join(search, u".."_S);
        }

        throw RuntimeError(u"Couldn't locate sample #.!"_S.format(filename));
    }

    virtual String path(const String &filename) = 0;
};

class EXSSamplePoolDummy : public EXSSamplePool
{
public:

    String name;

    template <typename T1 = decltype(u""_S)> EXSSamplePoolDummy(const T1 &name = u""_S) :
        name(name)
    {
    }

    virtual String path(const String &filename) override
    {
        return fs::path::join(name, filename);
    }
};

class EXSSamplePoolFixed : public EXSSamplePool
{
public:

    template <typename T1> EXSSamplePoolFixed(const T1 &path)
    {
        if (fs::is_dir(path))
            base = fs::path::dir_name(path);
        else
            throw RuntimeError(u"#.is not a valid path!"_S.format(path));
    }

    virtual String path(const String &filename) override
    {
        return fs::path::join(base, locate(filename), filename);
    }
};

class EXSSamplePoolLocator : public EXSSamplePool
{
public:

    template <typename T1> EXSSamplePoolLocator(const T1 &exsfile_name)
    {
        base = fs::path::dir_name(exsfile_name);
    }

    virtual String path(const String &filename) override
    {
        return fs::path::join(locate(filename, 4), filename);
    }
};

class EXSInstrument
{
public:

    std::unique_ptr<EXSSamplePool> pool;

    String exsfile_name;

    Array<EXSZone> zones;
    Array<EXSGroup> groups;
    Array<EXSSample> samples;

    template <typename T1, typename T2 = decltype(u""_S)> EXSInstrument(const T1 &exsfile_name, const T2 &sample_location = u""_S) :
        exsfile_name(exsfile_name)
    {

        if (fs::file_size(exsfile_name) > 1024 * 1024)
            throw RuntimeError(u"EXS file is too large; will not parse! (size > 1 MebiByte)"_S);
        ::instrument_data = File(exsfile_name).read_bytes();

        if (unpack_from_bytes_be<UInt32>(::instrument_data, 0) == EXSHeader_sig && ::instrument_data[range_el(16, 20)] == "SOBT"_B)
            throw RuntimeError(u"File is a big endian EXS file; cannot parse!"_S);
        if (!(unpack_from_bytes<UInt32>(::instrument_data, 0) == EXSHeader_sig) && ::instrument_data[range_el(16, 20)] == "TBOS"_B)
            throw RuntimeError(u"File is not an EXS file; will not parse!"_S);

        if (sample_location == u"")
            pool = std::make_unique<EXSSamplePoolLocator>(exsfile_name);
        else
            pool = std::make_unique<EXSSamplePoolFixed>(sample_location);

        auto offset = 0;
        auto end = ::instrument_data.len();
        while (offset < end) {
            auto sig = unpack_from_bytes<UInt32>(::instrument_data, offset);

            if (sig == EXSHeader_sig)
                auto t = EXSHeader(offset);
            else if (sig == 0x0100'0101)
                zones.append(EXSZone(offset));
            else if (sig == 0x0200'0101)
                groups.append(EXSGroup(offset));
            else if (sig == 0x0300'0101)
                samples.append(EXSSample(offset));
            else if (sig == 0x0400'0101)
                auto t = EXSParam(offset);
            else
                throw RuntimeError(u"Encountered an unknown chunk signature! signature is "_S & (u"0x"_S & hex(sig).lowercase()));

            offset += chunk_size(::instrument_data, offset);
        }
    }

    auto build_sequences()
    {
        uR"( exs handles round robin samples by using groups that point to the next group, and so on
			until the sequence is reset by pointing to group -1;
			here we trace each of those chains for simple processing later )"_S;

        Array<Array<int>> sequences;
        {int Lindex = 0;

        for (auto &&group : groups) {{
            auto gid = Lindex;
            if (!group.sequence())
                goto on_continue;
            {bool was_break = false;

            for (auto sequence : sequences)
                if (false) {
                    was_break = true;
                    break;
                }
            if (!was_break) {

                Array<int> sequence;

                auto cont = true;
                while (cont == true) {
                    cont = false;
                    {int Lindex = 0;
                    for (auto &&g : groups) {
                        auto gid2 = Lindex;
                        if (g.sequence() == gid && !(gid2 == g.sequence()) && !(in(gid, sequence))) {
                            sequence.append(gid);
                            gid = gid2;
                            cont = true;
                            break;
                        }
                        Lindex++;
                    }}
                }

                sequence.drop();
                while (!(gid == -1) && !(in(gid, sequence))) {
                    sequence.append(gid);
                    gid = groups[gid].sequence();
                }

                if (sequence.len() > 1)
                    sequences.append(sequence);
            }
            }
} on_continue:
            Lindex++;
        }}

        return sequences;
    }

    template <typename T1, typename T2 = decltype(false)> auto convert(const T1 &sfzfilename, const T2 &overwrite = false)
    {

        auto sequences = build_sequences();

        auto get_sequence_position = [&sequences](const auto &zone)
        {
            for (auto &&sequence : sequences)
                if (in(zone.group(), sequence))
                    return sequence.index(zone.group());
            return 0;
        };

        auto get_rootnote = [](const auto &zone)
        {
            if (!zone.pitchtrack()) {
                if (zone.rootnote() < zone.startnote() || zone.rootnote() > zone.endnote())
                    return zone.startnote();
            }
            return zone.rootnote();
        };

        Dict<Tuple<int, int, int, int, int>, Array<EXSZone>> ranges;
        for (auto &&zone : zones) {
            auto key = make_tuple(zone.startnote(), zone.endnote(), get_rootnote(zone), zone.pan(), get_sequence_position(zone));
            if (!(in(key, ranges))) {
                Array<EXSZone> empty_list;
                ranges.set(key, empty_list);
            }
            ranges[key].append(zone);
        }

        Dict<Tuple<int, int, int, int, int>, int> key_sequence;
        for (auto &&key : sorted(ranges.keys())) {

            auto keyrange = ranges[key];
            auto group = _get<0>(keyrange).group();
            {bool was_break = false;

            for (auto &&sequence : sequences)
                if (in(group, sequence)) {
                    group = _get<0>(sequence);
                    was_break = true;
                    break;
                }
            if (!was_break)
                continue;
            }

            key_sequence.set(key, group);
        }

        if (!overwrite && fs::is_file(sfzfilename))
            throw RuntimeError(u"file #. already exists; will not overwrite!"_S.format(sfzfilename));

        auto sfzfile = File(sfzfilename, u"w"_S);

        sfzfile.write(u"// this file was generated from #. using vonred's #.. Trickster goddess incoming.\n\n"_S.format(fs::path::base_name(exsfile_name), fs::path::base_name(u"1.exs2sfz.py"_S)));

        for (auto &&key : sorted(ranges.keys())) {
            auto keyrange = ranges[key];

            sfzfile.write(u"<group>"_S);

            if ((equal(_get<0>(key), _get<1>(key), _get<2>(key))))
                sfzfile.write(u" key=#."_S.format(_get<0>(key)));
            else
                sfzfile.write(u"lokey=#. hikey=#. pitch_keycenter=#."_S.format(_get<0>(keyrange).startnote(), _get<0>(keyrange).endnote(), _get<0>(keyrange).rootnote()));

            int choke_group;
            if (in(key, key_sequence))
                choke_group = key_sequence[key];
            else
                choke_group = _get<0>(keyrange).group();

            Dict<ivec2, int> choke_voices;
            for (auto &&zone : keyrange)
                if (in(make_tuple(zone.minvel(), zone.maxvel()), choke_voices))
                    choke_voices[make_tuple(zone.minvel(), zone.maxvel())]++;
                else
                    choke_voices.set(make_tuple(zone.minvel(), zone.maxvel()), 1);

            if (groups[_get<0>(keyrange).group()].polyphony() == choke_voices[make_tuple(_get<0>(keyrange).minvel(), _get<0>(keyrange).maxvel())]) {

                if (choke_group == 0)
                    choke_group = groups.len();

                sfzfile.write(u" group=#. off_by=#. polyphony=#. off_mode=fast"_S.format(choke_group, choke_group, groups[_get<0>(keyrange).group()].polyphony()));
            }

            if (groups[_get<0>(keyrange).group()].output() > 0)
                sfzfile.write(u" output=#."_S.format(groups[_get<0>(keyrange).group()].output()));

            if (_get<0>(keyrange).pan())
                sfzfile.write(u" pan=#."_S.format(_get<0>(keyrange).pan()));

            if (_get<0>(keyrange).oneshot())
                sfzfile.write(u" loop_mode=one_shot"_S);

            for (auto &&sequence : sequences)
                if (in(_get<0>(keyrange).group(), sequence)) {
                    sfzfile.write(u" seq_length=#. seq_position=#."_S.format(sequence.len(), sequence.index(_get<0>(keyrange).group()) + 1));
                    break;
                }

            sfzfile.write(u" pitch_keytrack=#.\n"_S.format(to_int(_get<0>(keyrange).pitchtrack())));

            for (auto &&zone : keyrange) {
                sfzfile.write(u"\t<region> lovel=#03 hivel=#03 amp_velcurve_#03=1"_S.format(zone.minvel(), zone.maxvel(), zone.maxvel()));

                if (zone.finetune())
                    sfzfile.write(u" tune=#."_S.format(zone.finetune()));

                if (zone.volumeadjust())
                    sfzfile.write(u" volume=#."_S.format(zone.volumeadjust()));

                if (zone.samplestart())
                    sfzfile.write(u" offset=#."_S.format(zone.samplestart()));

                if (zone.sampleend() && zone.sampleend() < samples[zone.sampleindex()].length())
                    sfzfile.write(u" end=#."_S.format(zone.sampleend()));

                if (zone._loop_())
                    sfzfile.write(u" loop_mode=loop_sustain loop_start=#. loop_end=#."_S.format(zone.loopstart(), zone.loopend() - 1));

                if (groups[zone.group()].trigger() == 1)
                    sfzfile.write(u" trigger=release"_S);

                sfzfile.write(u" sample=#."_S.format(pool->path(samples[zone.sampleindex()].name())));

                sfzfile.write(u"\n"_S);
            }
        }
    }
};

int MAIN_WITH_ARGV()
{
    INIT_ARGV();

    if (::argv.len() < 3 || ::argv.len() > 4) {
        print(u"Usage: #. EXSfile.exs SFZfile.sfz [samplefolder]"_S.format(u"1.exs2sfz.py"_S));
        print();
        print(u"    the samplefolder argument is optional; if not specified, the program will"_S);
        print(u"    attempt to locate the samples by searching folders surrounding the exs file"_S);
        print();

        exit(64);
    }

    auto samplefolder = ::argv.len() == 4 ? _get<3>(::argv) : u""_S;
    try
    {
        auto exs = EXSInstrument(_get<1>(::argv), samplefolder);
        exs.convert(_get<2>(::argv));
    }
    catch (const RuntimeError& rerr)
    {
        exit(rerr);
    }
}
