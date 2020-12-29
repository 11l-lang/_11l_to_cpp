#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

template <typename T1 = decltype(6)> auto generateRandomPoints(const T1 &npoints = 6)
{
    u"Generate a few random points v1...vn"_S;
    print(String(npoints) & u" points x,y:"_S);
    Array<dvec2> points;
    for (auto i : range_el(0, npoints)) {
        auto [xrand, yrand] = make_tuple(randomns::_(), randomns::_());
        print(String(xrand) & u" "_S & String(yrand));
        for (auto xoff : range_el(-1, 2))
            for (auto yoff : range_el(-1, 2))
                points.append(make_tuple(xrand + xoff, yrand + yoff));
    }
    return points;
}

template <typename T1, typename T2, typename T3> auto closest(const T1 &x, const T2 &y, const T3 &points)
{
    u"Function to find the closest of the vi."_S;
    auto [best, good] = make_tuple(99.0 * 99.0, 99.0 * 99.0);
    for (auto &&[px, py] : points) {
        auto dist = (x - px) * (x - px) + (y - py) * (y - py);
        if (dist < best)
            assign_from_tuple(best, good, make_tuple(dist, best));
        else if (dist < good)
            good = dist;
    }
    return sqrt(best) / sqrt(good);
}

template <typename T1, typename T2 = decltype(40), typename T3 = decltype(80)> auto generateScreen(const T1 &points, const T2 &rows = 40, const T3 &cols = 80)
{
    auto yfact = 1.0 / cols;
    auto xfact = 1.0 / rows;
    Array<String> screen;
    auto chars = u" -.,+*$&#~~"_S;
    for (auto i : range_el(0, rows)) {
        auto x = i * xfact;
        auto line = range_el(0, cols).map([&chars, &points, &x, &yfact](const auto &j){return chars[to_int(10 * closest(x, j * yfact, points))];});
        screen.extend(line);
        screen.append(u"\n"_S);
    }
    return screen.join(u""_S);
}

int main()
{
    auto points = generateRandomPoints(10);
    print();
    auto t1 = timens::perf_counter();
    print(generateScreen(points, 40, 80));
    auto t2 = timens::perf_counter();
    print(round(t2 - t1, 3));
}
