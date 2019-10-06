#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

template <typename T1, typename T2> auto cross(const T1 &aa, const T2 &bb)
{
    return multiloop(aa, bb, [](const auto &a, const auto &b){return a + b;});
}
auto rows = u"ABCDEFGHI"_S;
auto cols = u"123456789"_S;
auto digits = u"123456789"_S;
auto squares = cross(rows, cols);
auto unitlist = (cols.map([](const auto &c){return cross(::rows, String(c));}) + rows.map([](const auto &r){return cross(String(r), ::cols);}) + multiloop(create_array({u"ABC"_S, u"DEF"_S, u"GHI"_S}), create_array({u"123"_S, u"456"_S, u"789"_S}), [](const auto &rs, const auto &cs){return cross(rs, cs);}));
auto units = create_dict(squares.map([](const auto &s){return make_tuple(s, ::unitlist.filter([&s](const auto &u){return in(s, u);}));}));
Dict<String, Set<String>> peers;

struct CodeBlock1
{
    CodeBlock1()
    {
        for (auto &&s : squares) {
            auto se = Set<String>();
            for (auto &&u : units[s])
                for (auto &&s2 : u)
                    if (s2 != s)
                        se.add(s2);
            peers.set(s, se);
        }
    }
} code_block_1;

template <typename T1> Dict<String, String> search(const T1 &values)
{
    u"Using depth-first search and propagation, try all possible values."_S;
    if (values.empty())
        return Dict<String, String>();
    if (all(::squares.map([&values](const auto &s){return values[s].len() == 1;})))
        return values;
    auto s = _get<1>(min(::squares.filter([&values](const auto &s){return values[s].len() > 1;}).map([&values](const auto &s){return make_tuple(values[s].len(), s);})));
    for (auto &&d : values[s]) {
        auto values_copy = values.copy();
        auto r = search(assign(values_copy, s, d));
        if (!r.empty())
            return r;
    }
    return Dict<String, String>();
}

template <typename T2, typename T3> Dict<String, String> assign(Dict<String, String> &values, const T2 &s, const T3 &d)
{
    u"Eliminate all the other values (except d) from values[s] and propagate."_S;
    if (all(values[s].filter([&d](const auto &d2){return d2 != d;}).map([&s, &values](const auto &d2){return !eliminate(values, s, d2).empty();})))
        return values;
    else
        return Dict<String, String>();
}

template <typename T2, typename T3> Dict<String, String> eliminate(Dict<String, String> &values, const T2 &s, const T3 &d)
{
    u"Eliminate d from values[s]; propagate when values or places <= 2."_S;
    if (!in(d, values[s]))
        return values;
    values.set(s, values[s].replace(d, u""_S));
    if (values[s].empty())
        return Dict<String, String>();
    else if (values[s].len() == 1) {
        auto d2 = _get<0>(values[s]);
        if (!all(::peers[s].map([&d2, &values](const auto &s2){return !eliminate(values, s2, d2).empty();})))
            return Dict<String, String>();
    }

    for (auto &&u : ::units[s]) {
        auto dplaces = u.filter([&d, &values](const auto &s){return in(d, values[s]);});
        if (dplaces.empty())
            return Dict<String, String>();
        else if (dplaces.len() == 1) {
            if (assign(values, _get<0>(dplaces), d).empty())
                return Dict<String, String>();
        }
    }
    return values;
}

template <typename T1> Dict<String, String> parse_grid(T1 grid)
{
    u"Given a string of 81 digits (or .0-), return a dict of {cell:values}"_S;
    grid = grid.filter([](const auto &c){return in(c, u"0.-123456789"_S);});
    auto values = create_dict(::squares.map([](const auto &s){return make_tuple(s, ::digits);}));
    for (auto &&[s, d] : zip(::squares, grid))
        if (in(d, ::digits) && assign(values, s, d).empty())
            return Dict<String, String>();
    return values;
}

template <typename T1, typename T2, typename T3> auto solve_file(const T1 &filename, const T2 &sep, const T3 &action)
{
    u"Parse a file into a sequence of 81-char descriptions and solve them."_S;
    auto results = File(filename).read().trim(make_tuple(u" "_S, u"\t"_S, u"\r"_S, u"\n"_S)).split(sep).map([&action](const auto &grid){return action(search(parse_grid(grid)));});
    print(u"## Got #.0 out of #.0"_S.format(sum_map(results, [](const auto &r){return (!r.empty() ? 1 : 0);}), results.len()));
    return results;
}

template <typename T1> auto printboard(const T1 &values)
{
    u"Used for debugging."_S;
    auto width = 1 + max(::squares.map([&values](const auto &s){return values[s].len();}));
    auto line = u"\n"_S + (create_array({u"-"_S * (width * 3)}) * 3).join(u"+"_S);
    for (auto &&r : ::rows)
        print((::cols.map([&r, &values, &width](const auto &c){return values[r + c].center(width) + (in(c, u"36"_S) ? u"|"_S : u""_S);})).join(u""_S) + (in(r, u"CF"_S) ? line : u""_S));
    print();
    return values;
}

int main()
{
    solve_file(u"testdata/top95.txt"_S, u"\n"_S, [](const auto &v){return printboard(v);});
}
