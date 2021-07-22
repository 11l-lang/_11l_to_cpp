#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto FLOAT_MAX = 1e100;

class Point
{
public:
    decltype(0.0) x = 0.0;
    decltype(0.0) y = 0.0;
    decltype(0) group = 0;
};

template <typename T1, typename T2> auto generate_points(const T1 &npoints, const T2 &radius)
{
    auto points = range_el(0, npoints).map([](const auto &_){return Point();});

    for (auto &&p : points) {
        auto r = randomns::_() * radius;
        auto ang = randomns::_() * 2 * math::pi;
        p.x = r * cos(ang);
        p.y = r * sin(ang);
    }

    return points;
}

template <typename T1, typename T2> auto sqr_distance_2D(const T1 &a, const T2 &b)
{
    return square((a.x - b.x)) + square((a.y - b.y));
}

template <typename T1, typename T2> auto nearest_cluster_center(const T1 &point, const T2 &cluster_centers)
{
    u"Distance and index of the closest cluster center"_S;

    auto min_index = point.group;
    auto min_dist = ::FLOAT_MAX;
    {int Lindex = 0;

    for (auto &&cc : cluster_centers) {
        auto i = Lindex;
        auto d = sqr_distance_2D(cc, point);
        if (min_dist > d) {
            min_dist = d;
            min_index = i;
        }
        Lindex++;
    }}

    return make_tuple(min_index, min_dist);
}

template <typename T1, typename T2> auto kpp(T1 &points, T2 &cluster_centers)
{
    _set<0>(cluster_centers, copy(randomns::choice(points)));
    auto d = range_el(0, points.len()).map([](const auto &_){return 0.0;});

    for (auto i : range_el(1, cluster_centers.len())) {
        auto sum = 0.0;
        {int Lindex = 0;
        for (auto &&p : points) {
            auto j = Lindex;
            d.set(j, _get<1>(nearest_cluster_center(p, cluster_centers[range_el(0, i)])));
            sum += d[j];
            Lindex++;
        }}

        sum *= randomns::_();
        {int Lindex = 0;

        for (auto &&di : d) {{
            auto j = Lindex;
            sum -= di;
            if (sum > 0)
                goto on_continue;
            cluster_centers.set(i, copy(points[j]));
            break;
} on_continue:
            Lindex++;
        }}
    }
    for (auto &&p : points)
        p.group = _get<0>(nearest_cluster_center(p, cluster_centers));
}

template <typename T1, typename T2> auto lloyd(T1 &points, const T2 &nclusters)
{
    auto cluster_centers = range_el(0, nclusters).map([](const auto &_){return Point();});

    kpp(points, cluster_centers);

    auto lenpts10 = points.len() >> 10;

    auto changed = 0;
    while (true) {
        for (auto &&cc : cluster_centers) {
            cc.x = 0;
            cc.y = 0;
            cc.group = 0;
        }

        for (auto &&p : points) {
            cluster_centers[p.group].group++;
            cluster_centers[p.group].x += p.x;
            cluster_centers[p.group].y += p.y;
        }

        for (auto &&cc : cluster_centers) {
            cc.x /= cc.group;
            cc.y /= cc.group;
        }

        changed = 0;
        for (auto &&p : points) {
            auto min_i = _get<0>(nearest_cluster_center(p, cluster_centers));
            if (min_i != p.group) {
                changed++;
                p.group = min_i;
            }
        }
        if (changed <= lenpts10)
            break;
    }
    {int Lindex = 0;

    for (auto &&cc : cluster_centers) {
        auto i = Lindex;
        cc.group = i;
        Lindex++;
    }}

    return cluster_centers;
}

class Color
{
public:
    double r;
    double g;
    double b;
    template <typename T1, typename T2, typename T3> Color(const T1 &r, const T2 &g, const T3 &b) :
        r(r),
        g(g),
        b(b)
    {
    }
};

template <typename T1, typename T2, typename T3 = decltype(400), typename T4 = decltype(400)> auto print_eps(const T1 &points, const T2 &cluster_centers, const T3 &w = 400, const T4 &h = 400)
{
    Array<Color> colors;
    for (auto i : range_el(0, cluster_centers.len()))
        colors.append(Color((mod(3 * (i + 1), 11)) / 11.0, (mod(7 * i, 11)) / 11.0, (mod(9 * i, 11)) / 11.0));

    auto max_x = -::FLOAT_MAX;
    auto max_y = -::FLOAT_MAX;
    auto min_x = ::FLOAT_MAX;
    auto min_y = ::FLOAT_MAX;

    for (auto &&p : points) {
        if (max_x < p.x)
            max_x = p.x;
        if (min_x > p.x)
            min_x = p.x;
        if (max_y < p.y)
            max_y = p.y;
        if (min_y > p.y)
            min_y = p.y;
    }

    auto scale = min(w / (max_x - min_x), h / (max_y - min_y));
    auto cx = (max_x + min_x) / 2.0;
    auto cy = (max_y + min_y) / 2.0;

    print(u"%!PS-Adobe-3.0\n%%BoundingBox: -5 -5 #. #."_S.format(w + 10, h + 10));

    print((u"/l {rlineto} def /m {rmoveto} def\n"_S & u"/c { .25 sub exch .25 sub exch .5 0 360 arc fill } def\n"_S & u"/s { moveto -2 0 m 2 2 l 2 -2 l -2 -2 l closepath "_S & u"   gsave 1 setgray fill grestore gsave 3 setlinewidth"_S & u" 1 setgray stroke grestore 0 setgray stroke }def"_S));
    {int Lindex = 0;

    for (auto &&cc : cluster_centers) {
        auto i = Lindex;
        print((u"#. #. #. setrgbcolor"_S.format(colors[i].r, colors[i].g, colors[i].b)));

        for (auto &&p : points) {
            if (p.group != i)
                continue;
            print((u"#.3 #.3 c"_S.format((p.x - cx) * scale + w / 2.0, (p.y - cy) * scale + h / 2.0)));
        }

        print((u"\n0 setgray #. #. s"_S.format((cc.x - cx) * scale + w / 2.0, (cc.y - cy) * scale + h / 2.0)));
        Lindex++;
    }}

    print(u"\n%%%%EOF"_S);
}

auto _main_()
{
    auto npoints = 30000;
    auto k = 7;

    auto points = generate_points(npoints, 10);
    auto cluster_centers = lloyd(points, k);
    print_eps(points, cluster_centers);
}

struct CodeBlock1
{
    CodeBlock1()
    {
        _main_();
    }
} code_block_1;

int main()
{
}
