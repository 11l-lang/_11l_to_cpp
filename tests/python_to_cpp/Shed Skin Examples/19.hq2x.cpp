#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto LUT16to32 = 65536 * create_array({0});
auto RGBtoYUV = 65536 * create_array({0});
auto Ymask = 0x00FF'0000;
auto Umask = 0x0000'FF00;
auto Vmask = 0x0000'00FF;
auto trY = 0x0030'0000;
auto trU = 0x0000'0700;
auto trV = 0x0000'0006;

class PPM
{
public:
    int w;
    int h;
    Array<int> rgb;

    template <typename T1, typename T2, typename T3> PPM(const T1 &w, const T2 &h, const T3 &rgb) :
        rgb(rgb)
    {
        assign_from_tuple(this->w, this->h, make_tuple(w, h));
    }

    template <typename T1> auto save(const T1 &filename)
    {
        auto f = File(filename, u"w"_S);
        f.write(u"P3\n"_S);
        f.write(String(w) & u" "_S & String(h) & u"\n"_S);
        f.write(u"255\n"_S);
        for (auto &&rgb : this->rgb) {
            auto r = ((rgb >> 16) & 0xff);
            auto g = ((rgb >> 8) & 0xff);
            auto b = (rgb & 0xff);
            f.write(String(r) & u" "_S & String(g) & u" "_S & String(b) & u"\n"_S);
        }
        f.close();
    }
};

template <typename T1> auto loadPPM(const T1 &filename)
{
    auto lines = File(filename).read_lines(true).map([](const auto &l){return l.trim(make_tuple(u" "_S, u"\t"_S, u"\r"_S, u"\n"_S));});
    assert(_get<0>(lines) == u"P3");
    auto wh = _get<1>(lines).split_py().map([](const auto &i){return to_int(i);});
    auto w = _get<0>(wh);
    auto h = _get<1>(wh);
    assert(to_int(_get<2>(lines)) == 255);
    Array<int> values;
    for (auto &&line : lines[range_ei(3)])
        values.extend(line.split_py().map([](const auto &i){return to_int(i);}));
    Array<int> rgb;
    for (auto i : range_el(0, values.len()).step(3)) {
        auto r = values[i] >> 3;
        auto g = values[i + 1] >> 2;
        auto b = values[i + 2] >> 3;
        rgb.append(r << 11 | g << 5 | b);
    }
    return PPM(w, h, rgb);
}

template <typename T1, typename T2> auto diff(const T1 &w1, const T2 &w2)
{
    auto YUV1 = RGBtoYUV[w1];
    auto YUV2 = RGBtoYUV[w2];
    return (abs((YUV1 & Ymask) - (YUV2 & Ymask)) > ::trY) || (abs((YUV1 & Umask) - (YUV2 & Umask)) > ::trU) || (abs((YUV1 & Vmask) - (YUV2 & Vmask)) > ::trV);
}

template <typename T1, typename T2> auto Interp1(const T1 &c1, const T2 &c2)
{
    return (c1 * 3 + c2) >> 2;
}

template <typename T1, typename T2, typename T3> auto Interp2(const T1 &c1, const T2 &c2, const T3 &c3)
{
    return (c1 * 2 + c2 + c3) >> 2;
}

template <typename T1, typename T2, typename T3> auto Interp6(const T1 &c1, const T2 &c2, const T3 &c3)
{
    return ((((c1 & 0x0000'FF00) * 5 + (c2 & 0x0000'FF00) * 2 + (c3 & 0x0000'FF00)) & 0x0007'F800) + (((c1 & 0x00FF'00FF) * 5 + (c2 & 0x00FF'00FF) * 2 + (c3 & 0x00FF'00FF)) & 0x07F8'07F8)) >> 3;
}

template <typename T1, typename T2, typename T3> auto Interp7(const T1 &c1, const T2 &c2, const T3 &c3)
{
    return ((((c1 & 0x0000'FF00) * 6 + (c2 & 0x0000'FF00) + (c3 & 0x0000'FF00)) & 0x0007'F800) + (((c1 & 0x00FF'00FF) * 6 + (c2 & 0x00FF'00FF) + (c3 & 0x00FF'00FF)) & 0x07F8'07F8)) >> 3;
}

template <typename T1, typename T2, typename T3> auto Interp9(const T1 &c1, const T2 &c2, const T3 &c3)
{
    return ((((c1 & 0x0000'FF00) * 2 + ((c2 & 0x0000'FF00) + (c3 & 0x0000'FF00)) * 3) & 0x0007'F800) + (((c1 & 0x00FF'00FF) * 2 + ((c2 & 0x00FF'00FF) + (c3 & 0x00FF'00FF)) * 3) & 0x07F8'07F8)) >> 3;
}

template <typename T1, typename T2, typename T3> auto Interp10(const T1 &c1, const T2 &c2, const T3 &c3)
{
    return ((((c1 & 0x0000'FF00) * 14 + (c2 & 0x0000'FF00) + (c3 & 0x0000'FF00)) & 0x000F'F000) + (((c1 & 0x00FF'00FF) * 14 + (c2 & 0x00FF'00FF) + (c3 & 0x00FF'00FF)) & 0x0FF0'0FF0)) >> 4;
}

template <typename T2, typename T3, typename T4> auto PIXEL00_0(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, _get<5>(c));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_10(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp1(_get<5>(c), _get<1>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_11(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp1(_get<5>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_12(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp1(_get<5>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_20(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp2(_get<5>(c), _get<4>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_21(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp2(_get<5>(c), _get<1>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_22(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp2(_get<5>(c), _get<1>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_60(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp6(_get<5>(c), _get<2>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_61(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp6(_get<5>(c), _get<4>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_70(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp7(_get<5>(c), _get<4>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_90(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp9(_get<5>(c), _get<4>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL00_100(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut, Interp10(_get<5>(c), _get<4>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_0(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, _get<5>(c));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_10(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp1(_get<5>(c), _get<3>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_11(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp1(_get<5>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_12(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp1(_get<5>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_20(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp2(_get<5>(c), _get<2>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_21(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp2(_get<5>(c), _get<3>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_22(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp2(_get<5>(c), _get<3>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_60(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp6(_get<5>(c), _get<6>(c), _get<2>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_61(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp6(_get<5>(c), _get<2>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_70(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp7(_get<5>(c), _get<2>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_90(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp9(_get<5>(c), _get<2>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL01_100(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + 1, Interp10(_get<5>(c), _get<2>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_0(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, _get<5>(c));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_10(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp1(_get<5>(c), _get<7>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_11(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp1(_get<5>(c), _get<8>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_12(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp1(_get<5>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_20(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp2(_get<5>(c), _get<8>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_21(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp2(_get<5>(c), _get<7>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_22(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp2(_get<5>(c), _get<7>(c), _get<8>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_60(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp6(_get<5>(c), _get<4>(c), _get<8>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_61(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp6(_get<5>(c), _get<8>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_70(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp7(_get<5>(c), _get<8>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_90(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp9(_get<5>(c), _get<8>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL10_100(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL, Interp10(_get<5>(c), _get<8>(c), _get<4>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_0(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, _get<5>(c));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_10(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp1(_get<5>(c), _get<9>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_11(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp1(_get<5>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_12(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp1(_get<5>(c), _get<8>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_20(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp2(_get<5>(c), _get<6>(c), _get<8>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_21(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp2(_get<5>(c), _get<9>(c), _get<8>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_22(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp2(_get<5>(c), _get<9>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_60(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp6(_get<5>(c), _get<8>(c), _get<6>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_61(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp6(_get<5>(c), _get<6>(c), _get<8>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_70(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp7(_get<5>(c), _get<6>(c), _get<8>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_90(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp9(_get<5>(c), _get<6>(c), _get<8>(c)));
}
template <typename T2, typename T3, typename T4> auto PIXEL11_100(Array<int> &rgb_out, const T2 &pOut, const T3 &BpL, const T4 &c)
{
    rgb_out.set(pOut + BpL + 1, Interp10(_get<5>(c), _get<6>(c), _get<8>(c)));
}

template <typename T1, typename T2, typename T3> auto hq2x(const T1 &xres, const T2 &yres, const T3 &rgb)
{
    uR"(
    +--+--+--+
    |w1|w2|w3|
    +--+--+--+
    |w4|w5|w6|
    +--+--+--+
    |w7|w8|w9|
    +--+--+--+
    )"_S;
    auto c = 10 * create_array({0});
    auto w = 10 * create_array({0});
    auto rgb_out = 4 * rgb.len() * create_array({0});
    auto BpL = 2 * xres;

    for (auto j : range_el(0, yres)) {
        auto prevline = j > 0 ? -xres : 0;
        auto nextline = j < yres - 1 ? xres : 0;

        for (auto i : range_el(0, xres)) {
            auto pos = j * xres + i;
            auto pOut = j * xres * 4 + 2 * i;
            _set<3>(w, rgb[pos + prevline]);
            _set<2>(w, _get<3>(w));
            _set<1>(w, _get<3>(w));
            _set<6>(w, rgb[pos]);
            _set<5>(w, _get<6>(w));
            _set<4>(w, _get<6>(w));
            _set<9>(w, rgb[pos + nextline]);
            _set<8>(w, _get<9>(w));
            _set<7>(w, _get<9>(w));

            if (i > 0) {
                _set<1>(w, rgb[pos + prevline - 1]);
                _set<4>(w, rgb[pos - 1]);
                _set<7>(w, rgb[pos + nextline - 1]);
            }

            if (i < xres - 1) {
                _set<3>(w, rgb[pos + prevline + 1]);
                _set<6>(w, rgb[pos + 1]);
                _set<9>(w, rgb[pos + nextline + 1]);
            }
            auto pattern = 0;
            auto flag = 1;
            auto YUV1 = RGBtoYUV[_get<5>(w)];
            for (auto k : range_ee(1, 9)) {
                if (k == 5)
                    continue;
                if (w[k] != _get<5>(w)) {
                    auto YUV2 = RGBtoYUV[w[k]];
                    if ((abs((YUV1 & Ymask) - (YUV2 & Ymask)) > ::trY) || (abs((YUV1 & Umask) - (YUV2 & Umask)) > ::trU) || (abs((YUV1 & Vmask) - (YUV2 & Vmask)) > ::trV))
                        pattern |= flag;
                }
                flag <<= 1;
            }
            for (auto k : range_ee(1, 9))
                c.set(k, LUT16to32[w[k]]);

            if (pattern == 0 || pattern == 1 || pattern == 4 || pattern == 32 || pattern == 128 || pattern == 5 || pattern == 132 || pattern == 160 || pattern == 33 || pattern == 129 || pattern == 36 || pattern == 133 || pattern == 164 || pattern == 161 || pattern == 37 || pattern == 165) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 2 || pattern == 34 || pattern == 130 || pattern == 162) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 16 || pattern == 17 || pattern == 48 || pattern == 49) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 64 || pattern == 65 || pattern == 68 || pattern == 69) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 8 || pattern == 12 || pattern == 136 || pattern == 140) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 3 || pattern == 35 || pattern == 131 || pattern == 163) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 6 || pattern == 38 || pattern == 134 || pattern == 166) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 20 || pattern == 21 || pattern == 52 || pattern == 53) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 144 || pattern == 145 || pattern == 176 || pattern == 177) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 192 || pattern == 193 || pattern == 196 || pattern == 197) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 96 || pattern == 97 || pattern == 100 || pattern == 101) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 40 || pattern == 44 || pattern == 168 || pattern == 172) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 9 || pattern == 13 || pattern == 137 || pattern == 141) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 18 || pattern == 50) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 80 || pattern == 81) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 72 || pattern == 76) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 10 || pattern == 138) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 66) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 24) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 7 || pattern == 39 || pattern == 135) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 148 || pattern == 149 || pattern == 180) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 224 || pattern == 228 || pattern == 225) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 41 || pattern == 169 || pattern == 45) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 22 || pattern == 54) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 208 || pattern == 209) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 104 || pattern == 108) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 11 || pattern == 139) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 19 || pattern == 51) {
                if ((diff(_get<2>(w), _get<6>(w)))) {
                    PIXEL00_11(rgb_out, pOut, BpL, c);
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_60(rgb_out, pOut, BpL, c);
                    PIXEL01_90(rgb_out, pOut, BpL, c);
                }
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 146 || pattern == 178) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w)))) {
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                    PIXEL11_12(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL01_90(rgb_out, pOut, BpL, c);
                    PIXEL11_61(rgb_out, pOut, BpL, c);
                }
                PIXEL10_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 84 || pattern == 85) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w)))) {
                    PIXEL01_11(rgb_out, pOut, BpL, c);
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL01_60(rgb_out, pOut, BpL, c);
                    PIXEL11_90(rgb_out, pOut, BpL, c);
                }
                PIXEL10_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 112 || pattern == 113) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w)))) {
                    PIXEL10_12(rgb_out, pOut, BpL, c);
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL10_61(rgb_out, pOut, BpL, c);
                    PIXEL11_90(rgb_out, pOut, BpL, c);
                }
            }
            else if (pattern == 200 || pattern == 204) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w)))) {
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                    PIXEL11_11(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL10_90(rgb_out, pOut, BpL, c);
                    PIXEL11_60(rgb_out, pOut, BpL, c);
                }
            }
            else if (pattern == 73 || pattern == 77) {
                if ((diff(_get<8>(w), _get<4>(w)))) {
                    PIXEL00_12(rgb_out, pOut, BpL, c);
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_61(rgb_out, pOut, BpL, c);
                    PIXEL10_90(rgb_out, pOut, BpL, c);
                }
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 42 || pattern == 170) {
                if ((diff(_get<4>(w), _get<2>(w)))) {
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                    PIXEL10_11(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_90(rgb_out, pOut, BpL, c);
                    PIXEL10_60(rgb_out, pOut, BpL, c);
                }
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 14 || pattern == 142) {
                if ((diff(_get<4>(w), _get<2>(w)))) {
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                    PIXEL01_12(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_90(rgb_out, pOut, BpL, c);
                    PIXEL01_61(rgb_out, pOut, BpL, c);
                }
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 67) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 70) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 28) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 152) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 194) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 98) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 56) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 25) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 26 || pattern == 31) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 82 || pattern == 214) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 88 || pattern == 248) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 74 || pattern == 107) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 27) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_10(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 86) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 216) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 106) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 30) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 210) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_10(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 120) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 75) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_10(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 29) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 198) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 184) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 99) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 57) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 71) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 156) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 226) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 60) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 195) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 102) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 153) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 58) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 83) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 92) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 202) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 78) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 154) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 114) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 89) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 90) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 55 || pattern == 23) {
                if ((diff(_get<2>(w), _get<6>(w)))) {
                    PIXEL00_11(rgb_out, pOut, BpL, c);
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_60(rgb_out, pOut, BpL, c);
                    PIXEL01_90(rgb_out, pOut, BpL, c);
                }
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 182 || pattern == 150) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w)))) {
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                    PIXEL11_12(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL01_90(rgb_out, pOut, BpL, c);
                    PIXEL11_61(rgb_out, pOut, BpL, c);
                }
                PIXEL10_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 213 || pattern == 212) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w)))) {
                    PIXEL01_11(rgb_out, pOut, BpL, c);
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL01_60(rgb_out, pOut, BpL, c);
                    PIXEL11_90(rgb_out, pOut, BpL, c);
                }
                PIXEL10_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 241 || pattern == 240) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w)))) {
                    PIXEL10_12(rgb_out, pOut, BpL, c);
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL10_61(rgb_out, pOut, BpL, c);
                    PIXEL11_90(rgb_out, pOut, BpL, c);
                }
            }
            else if (pattern == 236 || pattern == 232) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w)))) {
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                    PIXEL11_11(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL10_90(rgb_out, pOut, BpL, c);
                    PIXEL11_60(rgb_out, pOut, BpL, c);
                }
            }
            else if (pattern == 109 || pattern == 105) {
                if ((diff(_get<8>(w), _get<4>(w)))) {
                    PIXEL00_12(rgb_out, pOut, BpL, c);
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_61(rgb_out, pOut, BpL, c);
                    PIXEL10_90(rgb_out, pOut, BpL, c);
                }
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 171 || pattern == 43) {
                if ((diff(_get<4>(w), _get<2>(w)))) {
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                    PIXEL10_11(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_90(rgb_out, pOut, BpL, c);
                    PIXEL10_60(rgb_out, pOut, BpL, c);
                }
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 143 || pattern == 15) {
                if ((diff(_get<4>(w), _get<2>(w)))) {
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                    PIXEL01_12(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_90(rgb_out, pOut, BpL, c);
                    PIXEL01_61(rgb_out, pOut, BpL, c);
                }
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 124) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 203) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_10(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 62) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 211) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_10(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 118) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 217) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 110) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 155) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_10(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 188) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 185) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 61) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 157) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 103) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 227) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 230) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 199) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 220) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 158) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 234) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 242) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 59) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 121) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 87) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 79) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 122) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 94) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 218) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 91) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 229) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 167) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 173) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 181) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 186) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }

            if (pattern == 115) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 93) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 206) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 205 || pattern == 201) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_70(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 174 || pattern == 46) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_70(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 179 || pattern == 147) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_70(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 117 || pattern == 116) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_10(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_70(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 189) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 231) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 126) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 219) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_10(rgb_out, pOut, BpL, c);
                PIXEL10_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 125) {
                if ((diff(_get<8>(w), _get<4>(w)))) {
                    PIXEL00_12(rgb_out, pOut, BpL, c);
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_61(rgb_out, pOut, BpL, c);
                    PIXEL10_90(rgb_out, pOut, BpL, c);
                }
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 221) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w)))) {
                    PIXEL01_11(rgb_out, pOut, BpL, c);
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL01_60(rgb_out, pOut, BpL, c);
                    PIXEL11_90(rgb_out, pOut, BpL, c);
                }
                PIXEL10_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 207) {
                if ((diff(_get<4>(w), _get<2>(w)))) {
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                    PIXEL01_12(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_90(rgb_out, pOut, BpL, c);
                    PIXEL01_61(rgb_out, pOut, BpL, c);
                }
                PIXEL10_10(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 238) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w)))) {
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                    PIXEL11_11(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL10_90(rgb_out, pOut, BpL, c);
                    PIXEL11_60(rgb_out, pOut, BpL, c);
                }
            }
            else if (pattern == 190) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w)))) {
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                    PIXEL11_12(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL01_90(rgb_out, pOut, BpL, c);
                    PIXEL11_61(rgb_out, pOut, BpL, c);
                }
                PIXEL10_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 187) {
                if ((diff(_get<4>(w), _get<2>(w)))) {
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                    PIXEL10_11(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_90(rgb_out, pOut, BpL, c);
                    PIXEL10_60(rgb_out, pOut, BpL, c);
                }
                PIXEL01_10(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 243) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                PIXEL01_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w)))) {
                    PIXEL10_12(rgb_out, pOut, BpL, c);
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL10_61(rgb_out, pOut, BpL, c);
                    PIXEL11_90(rgb_out, pOut, BpL, c);
                }
            }
            else if (pattern == 119) {
                if ((diff(_get<2>(w), _get<6>(w)))) {
                    PIXEL00_11(rgb_out, pOut, BpL, c);
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                }
                else {
                    PIXEL00_60(rgb_out, pOut, BpL, c);
                    PIXEL01_90(rgb_out, pOut, BpL, c);
                }
                PIXEL10_12(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 237 || pattern == 233) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_100(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 175 || pattern == 47) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_100(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 183 || pattern == 151) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_100(rgb_out, pOut, BpL, c);
                PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 245 || pattern == 244) {
                PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_100(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 250) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                PIXEL01_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 123) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 95) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_10(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 222) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 252) {
                PIXEL00_21(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_100(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 249) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_100(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 235) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_100(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 111) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_100(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_22(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 63) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_100(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_21(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 159) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_100(rgb_out, pOut, BpL, c);
                PIXEL10_22(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 215) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_100(rgb_out, pOut, BpL, c);
                PIXEL10_21(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 246) {
                PIXEL00_22(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_100(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 254) {
                PIXEL00_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_100(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 253) {
                PIXEL00_12(rgb_out, pOut, BpL, c);
                PIXEL01_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_100(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_100(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 251) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                PIXEL01_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_100(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 239) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_100(rgb_out, pOut, BpL, c);
                PIXEL01_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_100(rgb_out, pOut, BpL, c);
                PIXEL11_11(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 127) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_100(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_20(rgb_out, pOut, BpL, c);
                PIXEL11_10(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 191) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_100(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_100(rgb_out, pOut, BpL, c);
                PIXEL10_11(rgb_out, pOut, BpL, c);
                PIXEL11_12(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 223) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_20(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_100(rgb_out, pOut, BpL, c);
                PIXEL10_10(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_20(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 247) {
                PIXEL00_11(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_100(rgb_out, pOut, BpL, c);
                PIXEL10_12(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_100(rgb_out, pOut, BpL, c);
            }
            else if (pattern == 255) {
                if ((diff(_get<4>(w), _get<2>(w))))
                    PIXEL00_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL00_100(rgb_out, pOut, BpL, c);
                if ((diff(_get<2>(w), _get<6>(w))))
                    PIXEL01_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL01_100(rgb_out, pOut, BpL, c);
                if ((diff(_get<8>(w), _get<4>(w))))
                    PIXEL10_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL10_100(rgb_out, pOut, BpL, c);
                if ((diff(_get<6>(w), _get<8>(w))))
                    PIXEL11_0(rgb_out, pOut, BpL, c);
                else
                    PIXEL11_100(rgb_out, pOut, BpL, c);
            }
        }
    }
    return rgb_out;
}

auto init_LUTs()
{
    for (int i = 0; i < 65536; i++)
        LUT16to32.set(i, ((i & 0xF8'00) << 8) | ((i & 0x07'E0) << 5) | ((i & 0x00'1F) << 3));
    for (int i = 0; i < 32; i++)
        for (int j = 0; j < 64; j++)
            for (int k = 0; k < 32; k++) {
                auto r = i << 3;
                auto g = j << 2;
                auto b = k << 3;
                auto Y = (r + g + b) >> 2;
                auto u = 128 + ((r - b) >> 2);
                auto v = 128 + ((-r + 2 * g - b) >> 3);
                RGBtoYUV.set((i << 11) | (j << 5) | k, (Y << 16) | (u << 8) | v);
            }
}

int main()
{
    init_LUTs();
    print(u"scaling randam.ppm to randam2.ppm (100 times).."_S);
    auto ppm = loadPPM(u"testdata/randam.ppm"_S);
    Array<int> rgb;
    for (int i = 0; i < 100; i++)
        rgb = hq2x(ppm.w, ppm.h, ppm.rgb);
    PPM(2 * ppm.w, 2 * ppm.h, rgb).save(u"testdata/randam2.ppm"_S);
}
