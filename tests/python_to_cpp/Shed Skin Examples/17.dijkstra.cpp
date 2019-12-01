#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

uint32_t random_seed = 0;
auto rnd()
{
    ::random_seed = (1664525 * ::random_seed + 1013904223) & 0xFFFF'FFFF;
    return ::random_seed;
}

template <typename T1> uint32_t randint(const T1 &max)
{
    return (to_uint64(rnd()) * max) >> 32;
}
template <typename T1> auto randfloat(const T1 &max)
{
    return rnd() * (max / to_float(0x0001'0000'0000));
}
template <typename T1, typename T2> auto random_uniform(const T1 &min, const T2 &max)
{
    return randfloat(max - min) + min;
}

class Vertex
{
public:
    String name;
    template <typename T1 = decltype(u""_S)> Vertex(const T1 &name = u""_S) :
        name(name)
    {
    }

    template <typename T1> auto operator<(const T1 &v) const
    {
        return name < v.name;
    }

    template <typename T1> auto operator==(const T1 &v) const
    {
        return name == v.name;
    }

    auto __hash__()
    {
        return hash(name);
    }

    operator String() const
    {
        return name;
    }

    auto __repr__()
    {
        return name;
    }
};

class Edge
{
public:
    Vertex u;
    Vertex v;
    double d;
    template <typename T1, typename T2, typename T3> Edge(const T1 &u, const T2 &v, const T3 &d) :
        u(u),
        v(v),
        d(d)
    {
    }
    operator String() const
    {
        return u"[#. --#.2--> #.]"_S.format(u.name, d, v.name);
    }
};

class Graph
{
public:
    Array<Vertex> v;
    Array<Edge> e;
    Graph()
    {
        Array<Vertex> v;
        for (auto n : range_el(0, 100))
            v.append(Vertex(String(n + 1)));
        Array<Edge> e;
        for (auto n : range_el(0, 10 * v.len())) {
            auto ui = randint(v.len());
            auto u = v[ui];
            auto tv = Vertex(u""_S);
            while (true) {
                auto tvi = randint(v.len());
                tv = v[tvi];
                if (tvi != ui)
                    break;
            }
            e.append(Edge(u, tv, random_uniform(10, 100)));
        }
        this->v = v;
        this->e = e;
    }

    template <typename T1, typename T2> auto distance(const T1 &s, const T2 &ss)
    {
        double d;
        {bool was_break = false;
        for (auto &&edge : ::G.e.filter([&s, &ss](const auto &e){return e.u == s && e.v == _get<0>(ss);})) {
            d = edge.d;
            was_break = true;
            break;
        }
        if (!was_break)
            assert(false);
        }
        for (auto &&[u, v] : zip(ss[range_e_llen(0,  - 1)], ss[range_ei(1)]))
            {bool was_break = false;
            for (auto &&edge : ::G.e.filter([&u, &v](const auto &e){return e.u == u && e.v == v;})) {
                d += edge.d;
                was_break = true;
                break;
            }
            if (!was_break)
                assert(false);
            }
        return d;
    }
};
auto G = Graph();

template <typename T2> auto Extract_Min(Array<Vertex> &q, const T2 &d)
{
    auto m_is_none = true;
    auto m = Vertex(u""_S);
    auto md = 1e50;
    for (auto &&u : q)
        if (m_is_none) {
            m = u;
            m_is_none = false;
            md = d[u];
        }
        else
            if (d[u] < md) {
                md = d[u];
                m = u;
            }
    q.remove(m);
    return m;
}

template <typename T1, typename T2, typename T3> auto dijkstra(const T1 &g, const T2 &t, const T3 &s)
{
    Dict<Vertex, double> d;
    Dict<Vertex, Vertex> previous;
    for (auto &&v : g.v) {
        d.set(v, 1e50);
        previous.set(v, Vertex(u""_S));
    }
    d.set(s, 0);
    Array<Vertex> ss;
    auto Q = copy(g.v);

    while (!Q.empty()) {
        auto u = Extract_Min(Q, d);
        if (u == t)
            break;
        ss.append(u);
        for (auto &&edge : g.e.filter([&u](const auto &e){return e.u == u;}))
            if (d[u] + edge.d < d[edge.v]) {
                d.set(edge.v, d[u] + edge.d);
                previous.set(edge.v, u);
            }
    }
    ss.clear();
    auto u = t;
    while (previous[u].name != u"") {
        ss.insert(0, u);
        u = previous[u];
    }
    return ss;
}

int main()
{
    for (auto n : range_el(0, 100)) {
        G = Graph();
        auto si = randint(G.v.len());
        auto s = G.v[si];
        auto t = Vertex(u""_S);
        while (true) {
            auto ti = randint(G.v.len());
            t = G.v[ti];
            if (ti != si)
                break;
        }
        auto ss = dijkstra(G, t, s);
        if (!ss.empty()) {
            print(u"dijkstra #. ---> #.: #. #.9"_S.format(String(s), String(t), String(ss), G.distance(s, ss)));
            for (auto &&inter : ss[range_e_llen(0,  - 1)]) {
                auto S1 = dijkstra(G, t, inter);
                print(u"\t => dijkstra #. ---> #.: #. #.9"_S.format(String(inter), String(t), String(S1), G.distance(inter, S1)));
                if (S1 != ss[range_ei((ss.len() - S1.len()))])
                    print(u"************ ALARM! **************"_S);
            }
        }
    }
}
