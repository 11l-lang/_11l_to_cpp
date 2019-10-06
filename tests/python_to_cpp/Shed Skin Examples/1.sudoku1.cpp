#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

template <typename T1, typename T2, typename T3, typename T4> auto validMove(const T1 &puzzle, const T2 &x, const T3 &y, const T4 &number)
{
    auto blnOK = true;
    auto px = idiv(x, 3);
    auto py = idiv(y, 3);
    if (puzzle[x][y] != 0)
        blnOK = false;
    if (blnOK)
        for (auto i : range_el(0, 9))
            if (puzzle[i][y] == number)
                blnOK = false;
    if (blnOK)
        for (auto j : range_el(0, 9))
            if (puzzle[x][j] == number)
                blnOK = false;
    if (blnOK)
        for (auto i : range_el(0, 3))
            for (auto j : range_el(0, 3))
                if (puzzle[px * 3 + i][py * 3 + j] == number)
                    blnOK = false;
    return blnOK;
}

template <typename T1, typename T2, typename T3> auto findallMoves(const T1 &puzzle, const T2 &x, const T3 &y)
{
    Array<int> returnList;
    for (auto n : range_el(1, 10))
        if (validMove(puzzle, x, y, n))
            returnList.append(n);
    return returnList;
}

auto solvePuzzleStep(Array<Array<int>> &puzzle)
{
    auto isChanged = false;
    for (auto y : range_el(0, 9))
        for (auto x : range_el(0, 9))
            if (puzzle[x][y] == 0) {
                auto allMoves = findallMoves(puzzle, x, y);
                if (allMoves.len() == 1) {
                    puzzle[x].set(y, _get<0>(allMoves));
                    isChanged = true;
                }
            }
    return isChanged;
}

auto solvePuzzleSimple(Array<Array<int>> &puzzle)
{
    auto iterationCount = 0;
    while (solvePuzzleStep(puzzle) == true)
        iterationCount++;
}
Dict<int, bool> hashtable;

template <typename T1> auto calc_hash_of_list(const T1 &l)
{
    auto hash = 2166136261;
    for (auto &&e : l)
        hash = (16777619 * (hash ^ e)) & 0xFFFF'FFFF;
    return hash;
}

template <typename T1> auto calc_hash(const T1 &puzzle)
{
    auto hashcode = 0;
    for (auto c : range_el(0, 9))
        hashcode = hashcode * 17 + calc_hash_of_list(puzzle[c]);
    return hashcode;
}

template <typename T1> auto hash_add(const T1 &puzzle)
{
    ::hashtable.set(calc_hash(puzzle), true);
}

template <typename T1> auto hash_lookup(const T1 &puzzle)
{
    return in(calc_hash(puzzle), ::hashtable);
}
auto iterations = 0;

template <typename T2, typename T3, typename T4> auto perm(Array<Array<int>> &puzzle, const T2 &i, const T3 &j, T4 l, Array<ivec2> &u)
{
    ::iterations++;
    if (u.empty() && l.empty()) {
        print(u"Solved!"_S);
        printpuzzle(puzzle);
        print(u"iterations: "_S + String(::iterations));
        return true;
    }
    else
        if (l.empty()) {
            Array<Array<int>> puzzlebackup;
            for (auto c : range_el(0, 9))
                puzzlebackup.append(puzzle[c][range_ei(0)]);
            solvePuzzleSimple(puzzle);
            for (auto c : range_el(0, u.len()))
                if (!hash_lookup(puzzle)) {
                    int inew;
                    int jnew;
                    assign_from_tuple(inew, jnew, u.pop(c));
                    l = genMoveList(puzzle, inew, jnew);
                    if (perm(puzzle, inew, jnew, l, u))
                        return true;
                    else
                        hash_add(puzzle);
                    u.insert(c, make_tuple(inew, jnew));
                }
            for (auto y : range_el(0, 9))
                for (auto x : range_el(0, 9))
                    puzzle[x].set(y, puzzlebackup[x][y]);
            hash_add(puzzle);
            return false;
        }
        else {
            auto ii = i * 3;
            auto jj = j * 3;
            for (auto m : range_el(0, l.len()))
                for (auto y : range_el(0, 3))
                    for (auto x : range_el(0, 3))
                        if (validMove(puzzle, x + ii, y + jj, l[m])) {
                            puzzle[x + ii].set(y + jj, l[m]);
                            auto backup = l.pop(m);
                            if ((perm(puzzle, i, j, l, u)))
                                return true;
                            else
                                hash_add(puzzle);
                            l.insert(m, backup);
                            puzzle[x + ii].set(y + jj, 0);
                        }
            return false;
        }
}

template <typename T1, typename T2, typename T3> auto genMoveList(const T1 &puzzle, const T2 &i, const T3 &j)
{
    auto l = create_array(range_el(1, 10));
    for (auto y : range_el(0, 3))
        for (auto x : range_el(0, 3)) {
            auto p = puzzle[i * 3 + x][j * 3 + y];
            if (p != 0)
                l.remove(p);
        }
    return l;
}

template <typename T1> auto printpuzzle(const T1 &puzzle)
{
    for (auto x : range_el(0, 9)) {
        auto s = u" "_S;
        for (auto y : range_el(0, 9)) {
            switch (puzzle[x][y])
            {
            case 0:
                s += u"."_S;
                break;
            default:
                s += String(puzzle[x][y]);
                break;
            }
            s += u" "_S;
        }
        print(s);
    }
}

auto solve()
{
    auto puzzle = create_array({create_array({0, 9, 3, 0, 8, 0, 4, 0, 0}), create_array({0, 4, 0, 0, 3, 0, 0, 0, 0}), create_array({6, 0, 0, 0, 0, 9, 2, 0, 5}), create_array({3, 0, 0, 0, 0, 0, 0, 9, 0}), create_array({0, 2, 7, 0, 0, 0, 5, 1, 0}), create_array({0, 8, 0, 0, 0, 0, 0, 0, 4}), create_array({7, 0, 1, 6, 0, 0, 0, 0, 2}), create_array({0, 0, 0, 0, 7, 0, 0, 6, 0}), create_array({0, 0, 4, 0, 1, 0, 8, 5, 0})});
    Array<ivec2> u;
    Array<int> lcount;
    for (auto y : range_el(0, 3))
        for (auto x : range_el(0, 3)) {
            u.append(make_tuple(x, y));
            lcount.append(genMoveList(puzzle, x, y).len());
        }
    for (auto j : range_el(0, 9))
        for (auto i : range_el(j, 9))
            if (i != j) {
                if (lcount[i] < lcount[j]) {
                    assign_from_tuple(u[i], u[j], make_tuple(u[j], u[i]));
                    assign_from_tuple(lcount[i], lcount[j], make_tuple(lcount[j], lcount[i]));
                }
            }
    auto l = genMoveList(puzzle, 0, 0);
    perm(puzzle, 0, 0, l, u);
}

struct CodeBlock1
{
    CodeBlock1()
    {
        for (auto x : range_el(0, 30))
            solve();
    }
} code_block_1;

int main()
{
}
