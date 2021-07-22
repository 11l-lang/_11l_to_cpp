#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto _9_bytes = 9 * create_array({Byte(1)});

class bmp
{
public:
    Array<Byte> v;
    template <typename T1 = decltype(_9_bytes), typename T2 = decltype(-1)> bmp(const T1 &vals = _9_bytes, const T2 &n = -1)
    {
        v = vals[range_el(0, 9)];
        if (n >= 0)
            v.set(n, !v[n]);
    }
    template <typename T1> auto operator&(const T1 &other) const
    {
        return bmp(range_el(0, 9).map([&other, this](const auto &i){return Byte(v[i] && other.v[i]);}));
    }
    auto cnt()
    {
        return v.filter([](const auto &i){return i;}).len();
    }
};

template <typename T1> auto calc_hash_of_str(const T1 &s)
{
    auto hash = 2166136261;
    for (auto &&c : s)
        hash = (16777619 * (hash ^ c.code)) & 0xFFFF'FFFF;
    return hash;
}

class BoardRep
{
public:
    Array<Array<int>> __fields;
    template <typename T1> BoardRep(const T1 &board)
    {
        __fields = board.final;
    }
    auto __hash__()
    {
        auto rep = u""_S;
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                rep &= String(__fields[i][j]);
        return calc_hash_of_str(rep);
    }
    template <typename T1> auto operator<(const T1 &other) const
    {
        return __fields < other.__fields;
    }
};

auto board_notifyOnCompletion = true;
auto board_completeSearch = false;

class board
{
public:
    Array<Array<int>> final = create_array({create_array({0}) * 9}) * 9;
    Array<bmp> rows = 9 * create_array({bmp()});
    Array<bmp> cols = 9 * create_array({bmp()});
    Array<Array<bmp>> cels = create_array({create_array({bmp()}) * 3}) * 3;

    decltype(0) __turns = 0;
    decltype(0) __backtracks = 0;
    decltype(0.0) __starttime = 0.0;
    decltype(0.0) __endtime = 0.0;
    decltype(0) __status = 0;
    decltype(0) __maxdepth = 0;
    decltype(81) __openspaces = 81;

    decltype(Set<BoardRep>()) solutions = Set<BoardRep>();
    decltype(Set<BoardRep>()) examined = Set<BoardRep>();
    board()
    {
    }

    template <typename T1 = decltype(u""_S)> auto fread(T1 filename = u""_S)
    {
        if (filename == u"")
            filename = input(u"filename: "_S);
        auto f = File(filename, u"r"_S);
        auto lines = f.read_lines(true);
        for (int row = 0; row < 9; row++)
            for (auto digit : range_ee(1, 9)) {
                try
                {
                    setval(row, lines[row].index(String(digit)), digit);
                }
                catch (const ValueError&)
                {
                }
            }
    }

    template <typename T1, typename T2, typename T3> auto setval(const T1 &row, const T2 &col, const T3 &val)
    {
        final[row].set(col, val);
        __openspaces--;

        auto mask = bmp(_9_bytes, val - 1);
        rows.set(row, rows[row] & mask);
        cols.set(col, cols[col] & mask);
        auto cr = cell(row);
        auto cc = cell(col);
        cels[cr].set(cc, cels[cr][cc] & mask);
    }

    template <typename T1> auto cell(const T1 &num) const
    {
        return to_int(ceil((num + 1) / 3.0)) - 1;
    }

    auto to_str() const
    {
        auto ret = u""_S;
        for (int row = 0; row < 9; row++) {
            if (row == 3 || row == 6)
                ret &= (((3 * u"---"_S) & u"+"_S) * 3)[range_e_llen(0,  - 1)] & u"\n"_S;
            for (int col = 0; col < 9; col++) {
                if (col == 3 || col == 6)
                    ret &= u"|"_S;
                ret &= u" "_S & (final[row][col] != 0 ? String(final[row][col]) : u" "_S) & u" "_S;
            }
            ret &= u"\n"_S;
        }
        return ret;
    }

    template <typename T1 = decltype(true), typename T2 = decltype(false)> auto solve(const T1 &notify = true, const T2 &completeSearch = false)
    {
        if (__status == 0) {
            __status = 1;
            __starttime = Time().unix_time();
            auto board_notifyOnCompletion = notify;
            auto board_completeSearch = completeSearch;
            __solve(*this, 0);
        }
    }

    auto openspaces() const
    {
        return __openspaces;
    }

    template <typename T1, typename T2> void __solve(const T1 &_board, const T2 &depth)
    {
        if (!in(BoardRep(_board), examined)) {
            examined.add(BoardRep(_board));

            if (_board.openspaces() <= 0) {
                solutions.add(BoardRep(_board));
                print(u"solution:"_S);
                print(_board.to_str());
                if (depth == 0)
                    onexit();
                if (!::board_completeSearch)
                    onexit();
            }

            else {
                __turns++;
                if (depth > __maxdepth)
                    __maxdepth = depth;

                int mincnt;
                Array<ivec2> coords;
                assign_from_tuple(mincnt, coords, _board.findmincounts());
                if (mincnt <= 0) {
                    __backtracks++;
                    if (depth == 0)
                        onexit();
                }
                else
                    for (auto &&[row, col] : coords) {
                        auto broken = false;
                        for (auto val : range_el(0, 9).filter([&_board, &col, &row](const auto &i){return _board.mergemask(row, col).v[i] == Byte(1);})) {
                            if (!::board_completeSearch && __status == 2) {
                                broken = true;
                                break;
                            }
                            val++;
                            auto t = _board.clone();
                            t.setval(row, col, val);
                            __solve(t, depth + 1);
                        }
                        if (broken)
                            break;
                    }
            }
        }
    }

    auto clone() const
    {
        auto ret = board();
        for (int row = 0; row < 9; row++)
            for (int col = 0; col < 9; col++)
                if (final[row][col])
                    ret.setval(row, col, final[row][col]);
        return ret;
    }

    template <typename T1, typename T2> auto mergemask(const T1 &row, const T2 &col) const
    {
        return rows[row] & cols[col] & cels[cell(row)][cell(col)];
    }

    auto findmincounts() const
    {
        Array<ivec3> masks;
        for (int row = 0; row < 9; row++)
            for (int col = 0; col < 9; col++)
                if (final[row][col] == 0) {
                    auto numallowed = mergemask(row, col).cnt();
                    masks.append(make_tuple(numallowed, row, col));
                }
        return make_tuple(_get<0>(min(masks)), masks.filter([&masks](const auto &i){return _get<0>(i) == _get<0>(min(masks));}).map([](const auto &i){return make_tuple(_get<1>(i), _get<2>(i));}));
    }

    auto stats()
    {
        auto t = __status == 1 ? Time().unix_time() - __starttime : __endtime - __starttime;
        return create_dict(dict_of(u"max depth"_S, __maxdepth)(u"turns"_S, __turns)(u"backtracks"_S, __backtracks)(u"elapsed time"_S, to_int(t))(u"boards examined"_S, examined.len())(u"number of solutions"_S, solutions.len()));
    }

    auto onexit()
    {
        __endtime = Time().unix_time();
        __status = 2;

        if (::board_notifyOnCompletion)
            print(stats()[u"turns"_S]);
    }
};

int main()
{
    auto puzzle = board();
    puzzle.fread(u"testdata/b6.pz"_S);
    print(puzzle.to_str());
    puzzle.solve();
}
