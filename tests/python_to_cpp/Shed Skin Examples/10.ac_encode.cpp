#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto BETA0 = 1;
auto BETA1 = 1;
auto M = 30;
auto ONE = (1 << M);
auto HALF = (1 << (M - 1));
auto QUARTER = (1 << (M - 2));
auto THREEQU = HALF + QUARTER;
template <typename T1> auto clear(const T1 &c, Array<int> &charstack)
{
    auto a = String(c) & String(1 - c) * _get<0>(charstack);
    _set<0>(charstack, 0);
    return a;
}

template <typename T1, typename T2 = decltype(BETA0), typename T3 = decltype(BETA1), typename T4 = decltype(1), typename T5 = decltype(0)> auto encode(const T1 &string, T2 c0 = BETA0, T3 c1 = BETA1, const T4 &adaptive = 1, const T5 &verbose = 0)
{
    auto b = ::ONE;
    auto a = 0;
    auto tot0 = 0;
    auto tot1 = 0;
    assert(c0 > 0);
    assert(c1 > 0);
    double p0;
    if (adaptive == 0)
        p0 = c0 * 1.0 / (c0 + c1);
    auto ans = u""_S;
    auto charstack = create_array({0});
    for (auto &&c : string) {
        auto w = b - a;
        if (adaptive) {
            auto cT = c0 + c1;
            p0 = c0 * 1.0 / cT;
        }
        auto boundary = a + to_int(p0 * w);
        if ((boundary == a)) {
            boundary++;
            print(u"warningA"_S);
        }
        if ((boundary == b)) {
            boundary--;
            print(u"warningB"_S);
        }

        if ((c == u'1')) {
            a = boundary;
            tot1++;
            if (adaptive)
                c1++;
        }
        else if ((c == u'0')) {
            b = boundary;
            tot0++;
            if (adaptive)
                c0++;
        }

        while (((a >= ::HALF) || (b <= ::HALF))) {
            if ((a >= ::HALF)) {
                ans = ans & clear(1, charstack);
                a = a - ::HALF;
                b = b - ::HALF;
            }
            else
                ans = ans & clear(0, charstack);
            a *= 2;
            b *= 2;
        }
        assert(a <= ::HALF);
        assert(b >= ::HALF);
        assert(a >= 0);
        assert(b <= ::ONE);

        while (((a > ::QUARTER) && (b < ::THREEQU))) {
            _get<0>(charstack)++;
            a = 2 * a - ::HALF;
            b = 2 * b - ::HALF;
        }
        assert(a <= ::HALF);
        assert(b >= ::HALF);
        assert(a >= 0);
        assert(b <= ::ONE);
    }

    if (((::HALF - a) > (b - ::HALF))) {
        auto w = (::HALF - a);
        ans = ans & clear(0, charstack);
        while ((w < ::HALF)) {
            ans = ans & clear(1, charstack);
            w *= 2;
        }
    }
    else {
        auto w = (b - ::HALF);
        ans = ans & clear(1, charstack);
        while ((w < ::HALF)) {
            ans = ans & clear(0, charstack);
            w *= 2;
        }
    }
    return ans;
}

template <typename T1, typename T2 = decltype(10000), typename T3 = decltype(BETA0), typename T4 = decltype(BETA1), typename T5 = decltype(1), typename T6 = decltype(0)> auto decode(const T1 &string, T2 n = 10000, T3 c0 = BETA0, T4 c1 = BETA1, const T5 &adaptive = 1, const T6 &verbose = 0)
{
    auto b = ::ONE;
    auto a = 0;
    auto tot0 = 0;
    auto tot1 = 0;
    assert(c0 > 0);
    assert(c1 > 0);
    auto model_needs_updating = 1;
    double p0;
    if (adaptive == 0)
        p0 = c0 * 1.0 / (c0 + c1);
    auto ans = u""_S;
    auto u = 0.0;
    auto v = to_float(::ONE);
    int boundary;
    for (auto &&c : string) {
        if (n <= 0)
            break;
        assert(n > 0);
        assert(u >= 0);
        assert(v <= ::ONE);
        auto halfway = u + (v - u) / 2.0;
        if ((c == u'1'))
            u = halfway;
        else if ((c == u'0'))
            v = halfway;

        while ((1)) {
            auto firsttime = 0;
            if ((model_needs_updating)) {
                auto w = b - a;
                if (adaptive) {
                    auto cT = c0 + c1;
                    p0 = c0 * 1.0 / cT;
                }
                boundary = a + to_int(p0 * w);
                if ((boundary == a)) {
                    boundary++;
                    print(u"warningA"_S);
                }
                if ((boundary == b)) {
                    boundary--;
                    print(u"warningB"_S);
                }
                model_needs_updating = 0;
            }
            if ((boundary <= u)) {
                ans = ans & u"1"_S;
                tot1++;
                if (adaptive)
                    c1++;
                a = boundary;
                model_needs_updating = 1;
                n--;
            }
            else if ((boundary >= v)) {
                ans = ans & u"0"_S;
                tot0++;
                if (adaptive)
                    c0++;
                b = boundary;
                model_needs_updating = 1;
                n--;
            }

            while (((a >= ::HALF) || (b <= ::HALF))) {
                if ((a >= ::HALF)) {
                    a = a - ::HALF;
                    b = b - ::HALF;
                    u = u - ::HALF;
                    v = v - ::HALF;
                }
                a *= 2;
                b *= 2;
                u *= 2;
                v *= 2;
                model_needs_updating = 1;
            }
            assert(a <= ::HALF);
            assert(b >= ::HALF);
            assert(a >= 0);
            assert(b <= ::ONE);

            while (((a > ::QUARTER) && (b < ::THREEQU))) {
                a = 2 * a - ::HALF;
                b = 2 * b - ::HALF;
                u = 2 * u - ::HALF;
                v = 2 * v - ::HALF;
            }
            if (!(n > 0 && model_needs_updating))
                break;
        }
    }
    return ans;
}

auto hardertest()
{
    print(u"Reading the BentCoinFile"_S);
    auto inputfile = File(u"testdata/BentCoinFile"_S, u"r"_S);
    auto outputfile = File(u"tmp.zip"_S, u"w"_S);
    print(u"Compressing to tmp.zip"_S);

    auto s = inputfile.read();
    auto n = s.len();
    auto zip = encode(s, 10, 1);
    outputfile.write(zip);
    outputfile.close();
    inputfile.close();
    print(u"DONE compressing"_S);

    inputfile = File(u"tmp.zip"_S, u"r"_S);
    outputfile = File(u"tmp2"_S, u"w"_S);
    print(u"Uncompressing to tmp2"_S);
    auto unc = decode(create_array(inputfile.read()), n, 10, 1);
    outputfile.write(unc);
    outputfile.close();
    inputfile.close();
    print(u"DONE uncompressing"_S);

    print(u"Checking for differences..."_S);
    print(s == unc);
}

auto test()
{
    auto sl = create_array({u"1010"_S, u"111"_S, u"00001000000000000000"_S, u"1"_S, u"10"_S, u"01"_S, u"0"_S, u"0000000"_S, u"000000000000000100000000000000000000000000000000100000000000000000011000000"_S});
    for (auto &&s : sl) {
        print(u"encoding "_S & s);
        auto n = s.len();
        auto e = encode(s, 10, 1);
        print(u"decoding "_S & e);
        auto ds = decode(e, n, 10, 1);
        print(ds);
        if ((ds != s)) {
            print(s);
            print(u"ERR@"_S);
        }
        else
            print(u"ok ---------- "_S);
    }
}

int main()
{
    test();
    hardertest();
}
