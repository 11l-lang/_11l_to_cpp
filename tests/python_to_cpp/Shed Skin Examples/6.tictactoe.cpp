#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

template <typename T1> auto sigmoid(const T1 &x)
{
    return to_float(1) / (1 + exp(-x));
}

template <typename T1, typename T2 = decltype(0), typename T3 = decltype(1)> auto sig(const T1 &x, const T2 &xshift = 0, const T3 &xcompress = 1)
{
    return 0 + (1 * sigmoid(xcompress * (x - xshift)));
}

class SpaceNotEmpty
{
};


class MultiVictory
{
public:
    Set<int> victors;
    template <typename T1> MultiVictory(const T1 &victorslist) :
        victors(victorslist)
    {
    }
};

class rectBoard
{
public:
    int edge;
    Array<Array<int>> __board;
    int __empty;

    template <typename T1 = decltype(3)> rectBoard(const T1 &edge = 3) :
        edge(edge)
    {
        __board = range_el(0, edge).map([&edge](const auto &i){return edge * create_array({0});});
        __empty = square(edge);
    }

    template <typename T1, typename T2, typename T3> auto assign(const T1 &row, const T2 &col, const T3 &value)
    {
        if ((__board[row][col] == 0)) {
            __board[row].set(col, value);
            __empty--;
        }
        else
            throw SpaceNotEmpty();
    }

    auto isfull()
    {
        return __empty == 0;
    }

    auto isvictory()
    {
        Array<int> victors;
        for (auto &&row : __board)
            if (create_set(row).len() == 1) {
                if (_get<0>(row) != 0)
                    victors.append(_get<0>(row));
            }

        for (auto i : range_el(0, edge)) {
            auto col = __board.map([&i](const auto &row){return row[i];});
            if (create_set(col).len() == 1) {
                if (_get<0>(col) != 0)
                    victors.append(_get<0>(col));
            }
        }
        Array<int> ld;
        for (auto i : range_el(0, edge))
            ld.append(__board[i][i]);
        if (create_set(ld).len() == 1) {
            if (_get<0>(ld) != 0)
                victors.append(_get<0>(ld));
        }
        Array<int> rd;
        for (auto i : range_el(0, edge))
            rd.append(__board[i][edge - (1 + i)]);
        if (create_set(rd).len() == 1) {
            if (_get<0>(rd) != 0)
                victors.append(_get<0>(rd));
        }
        if (victors.empty())
            return 0;
        if (create_set(victors).len() > 1)
            throw MultiVictory(create_set(victors));
        return _get<0>(victors);
    }

    operator String() const
    {
        auto ret = u""_S;
        for (auto row : range_el(0, edge)) {
            if (row != 0) {
                ret += u"\n"_S;
                for (auto i : range_el(0, edge)) {
                    if (i != 0)
                        ret += u"+"_S;
                    ret += u"---"_S;
                }
                ret += u"\n"_S;
            }
            ret += u" "_S;
            for (auto col : range_el(0, edge)) {
                if (col != 0)
                    ret += u" | "_S;
                if (__board[row][col] == 0)
                    ret += u" "_S;
                else
                    ret += String(__board[row][col]);
            }
        }
        return ret;
    }

    template <typename T1, typename T2, typename T3> auto doRow(const T1 &fields, const T2 &indices, const T3 &player, Array<Array<double>> &scores)
    {
        auto players = create_set(fields).difference(create_set({0}));

        if ((players.len() == 1)) {
            if (_get<0>(create_array(players)) == player)
                for (auto &&[rown, coln] : indices)
                    scores[rown][coln] += 15 * sig(fields.count(player) / to_float(edge), 0.5, 10);
            else
                for (auto &&[rown, coln] : indices)
                    scores[rown][coln] += 15 * fields.count(_get<0>(create_array(players))) / to_float(edge);
        }
    }

    template <typename T1> auto makeAImove(const T1 &player)
    {
        auto scores = range_el(0, edge).map([this](const auto &i){return edge * create_array({0.0});});

        for (auto rown : range_el(0, edge)) {
            auto row = __board[rown];
            doRow(row, range_el(0, edge).map([&rown](const auto &i){return make_tuple(rown, i);}), player, scores);
        }

        for (auto coln : range_el(0, edge)) {
            auto col = __board.map([&coln](const auto &row){return row[coln];});
            doRow(col, range_el(0, edge).map([&coln](const auto &i){return make_tuple(i, coln);}), player, scores);
        }
        auto indices = range_el(0, edge).map([](const auto &i){return make_tuple(i, i);});
        auto ld = range_el(0, edge).map([this](const auto &i){return __board[i][i];});
        doRow(ld, indices, player, scores);
        for (auto &&[rown, coln] : indices)
            scores[rown][coln]++;
        indices = range_el(0, edge).map([this](const auto &i){return make_tuple(i, (edge - 1) - i);});
        auto rd = range_el(0, edge).map([this](const auto &i){return __board[i][(edge - 1) - i];});
        doRow(rd, indices, player, scores);
        for (auto &&[rown, coln] : indices)
            scores[rown][coln]++;
        Array<Tuple<double, ivec2>> scorelist;
        for (auto rown : range_el(0, edge))
            for (auto coln : range_el(0, edge))
                if ((__board[rown][coln] == 0))
                    scorelist.append(make_tuple(scores[rown][coln], make_tuple(rown, coln)));
        scorelist.sort();
        scorelist.reverse();
        scorelist = scorelist.filter([&scorelist](const auto &x){return _get<0>(x) == _get<0>(_get<0>(scorelist));});
        return make_tuple(_get<0>(_get<1>(_get<0>(scorelist))), _get<1>(_get<1>(_get<0>(scorelist))));
    }
};

template <typename T1 = decltype(10), typename T2 = decltype(1), typename T3 = decltype(2)> auto aigame(const T1 &size = 10, T2 turn = 1, const T3 &players = 2)
{
    auto b = rectBoard(size);
    while (((!b.isfull()) && (b.isvictory() == 0)))
        if ((turn == 1)) {
            auto [r, c] = b.makeAImove(turn);
            b.assign(r, c, 1);
            turn = 2;
        }
        else {
            auto [r, c] = b.makeAImove(turn);
            b.assign(r, c, turn);
            if ((turn == players))
                turn = 1;
            else
                turn++;
        }
    print();
    print(b);
    print();
    if ((b.isvictory() == 0))
        print(u"Board is full! Draw!"_S);
    else
        print(u"Victory for player "_S + String(b.isvictory()) + u"!"_S);
}

int main()
{
    aigame();
}
