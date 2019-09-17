#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto TRIPLETS = create_array({create_array({0, 1, 2}), create_array({3, 4, 5}), create_array({6, 7, 8})});
auto ROW_ITER = range_el(0, 9).map([](const auto &row){return range_el(0, 9).map([&row](const auto &col){return make_tuple(row, col);});});
auto COL_ITER = range_el(0, 9).map([](const auto &col){return range_el(0, 9).map([&col](const auto &row){return make_tuple(row, col);});});
auto TxT_ITER = multiloop(TRIPLETS, TRIPLETS, [](const auto &rows, const auto &cols){return multiloop(rows, cols, [](const auto &row, const auto &col){return make_tuple(row, col);});});

class soduko
{
public:
    Array<Array<Array<int>>> squares = range_el(0, 9).map([](const auto &row){return range_el(0, 9).map([](const auto &col){return create_array(range_el(1, 10));});});
    decltype(false) _changed = false;
    template <typename T1> soduko(const T1 &start_grid)
    {
        if (!start_grid.empty()) {
            assert(start_grid.len() == 9, u"Bad input!"_S);
            for (auto row : range_el(0, 9))
                set_row(row, start_grid[row]);
        }
    }

    auto copy()
    {
        Array<String> e;
        auto soduko_copy = soduko(e);
        for (auto row : range_el(0, 9))
            for (auto col : range_el(0, 9))
                soduko_copy.squares[row].set(col, ::copy(squares[row][col]));
        soduko_copy._changed = false;
        return soduko_copy;
    }

    template <typename T1, typename T2> auto set_row(const T1 &row, const T2 &x_list)
    {
        assert(x_list.len() == 9, u"not 9"_S);
        for (auto col : range_el(0, 9)) {
            int x;
            try
            {
                x = to_int(x_list[col]);
            }
            catch (...)
            {
                x = 0;
            }
            set_cell(row, col, x);
        }
    }

    template <typename T1, typename T2, typename T3> auto set_cell(const T1 &row, const T2 &col, const T3 &x)
    {
        if (squares[row][col] == create_array({x})) {
        }

        else if (!in(x, range_el(1, 9 + 1))) {
        }

        else {
            assert(in(x, squares[row][col]), u"bugger2"_S);
            squares[row].set(col, create_array({x}));
            update_neighbours(row, col, x);
            _changed = true;
        }
    }

    class Error
    {
    };

    template <typename T1, typename T2, typename T3> auto cell_exclude(const T1 &row, const T2 &col, const T3 &x)
    {
        assert(in(x, range_el(1, 9 + 1)), u"inra"_S);
        if (in(x, squares[row][col])) {
            squares[row][col].remove(x);
            if (squares[row][col].empty())
                throw Error();
            if (squares[row][col].len() == 1) {
                _changed = true;
                update_neighbours(row, col, _get<0>(squares[row][col]));
            }
        }
        else {
        }
        return;
    }

    template <typename T1, typename T2, typename T3> void update_neighbours(const T1 &set_row, const T2 &set_col, const T3 &x)
    {
        for (auto row : range_el(0, 9))
            if (row != set_row)
                cell_exclude(row, set_col, x);
        for (auto col : range_el(0, 9))
            if (col != set_col)
                cell_exclude(set_row, col, x);
        Array<int> rows;
        Array<int> cols;
        for (auto &&triplet : ::TRIPLETS) {
            if (in(set_row, triplet))
                rows = ::copy(triplet);
            if (in(set_col, triplet))
                cols = ::copy(triplet);
        }
        rows.remove(set_row);
        cols.remove(set_col);
        for (auto &&row : rows)
            for (auto &&col : cols) {
                assert(row != set_row || col != set_col, u"meuh"_S);
                cell_exclude(row, col, x);
            }
    }

    template <typename T1, typename T2> auto get_cell_digit_str(const T1 &row, const T2 &col)
    {
        if (squares[row][col].len() == 1)
            return String(_get<0>(squares[row][col]));
        else
            return u"0"_S;
    }

    operator String()
    {
        auto answer = u"   123   456   789\n"_S;
        for (auto row : range_el(0, 9)) {
            answer = answer + String(row + 1) + u" ["_S + (range_el(0, 3).map([&row, this](const auto &col){return get_cell_digit_str(row, col).replace(u"0"_S, u"?"_S);})).join(u""_S) + u"] ["_S + (range_el(3, 6).map([&row, this](const auto &col){return get_cell_digit_str(row, col).replace(u"0"_S, u"?"_S);})).join(u""_S) + u"] ["_S + (range_el(6, 9).map([&row, this](const auto &col){return get_cell_digit_str(row, col).replace(u"0"_S, u"?"_S);})).join(u""_S) + u"]\n"_S;
            if (in(row + 1, create_array({3, 6})))
                answer = answer + u"   ---   ---   ---\n"_S;
        }
        return answer;
    }

    auto check_for_single_occurances()
    {
        for (auto &&check_type : create_array({::ROW_ITER, ::COL_ITER, TxT_ITER}))
            for (auto &&check_list : check_type)
                for (auto x : range_el(1, 9 + 1)) {
                    Array<ivec2> x_in_list;
                    for (auto &&[row, col] : check_list)
                        if (in(x, squares[row][col]))
                            x_in_list.append(make_tuple(row, col));
                    if (x_in_list.len() == 1) {
                        auto [row, col] = _get<0>(x_in_list);
                        if (squares[row][col].len() > 1)
                            set_cell(row, col, x);
                    }
                }
    }

    auto check_for_last_in_row_col_3x3()
    {
        for (auto &&[type_name, check_type] : create_array({make_tuple(u"Row"_S, ::ROW_ITER), make_tuple(u"Col"_S, ::COL_ITER), make_tuple(u"3x3"_S, TxT_ITER)}))
            for (auto &&check_list : check_type) {
                Array<ivec2> unknown_entries;
                auto unassigned_values = create_array(range_el(1, 9 + 1));
                Array<int> known_values;
                for (auto &&[row, col] : check_list)
                    if (squares[row][col].len() == 1) {
                        assert(!in(_get<0>(squares[row][col]), known_values), u"bugger3"_S);
                        known_values.append(_get<0>(squares[row][col]));
                        assert(in(_get<0>(squares[row][col]), unassigned_values), u"bugger4"_S);
                        unassigned_values.remove(_get<0>(squares[row][col]));
                    }
                    else
                        unknown_entries.append(make_tuple(row, col));
                assert(unknown_entries.len() + known_values.len() == 9, u"bugger5"_S);
                assert(unknown_entries.len() == unassigned_values.len(), u"bugger6"_S);
                if (unknown_entries.len() == 1) {
                    auto x = _get<0>(unassigned_values);
                    auto [row, col] = _get<0>(unknown_entries);
                    set_cell(row, col, x);
                }
            }
        return;
    }

    auto check()
    {
        _changed = true;
        while (_changed) {
            _changed = false;
            check_for_single_occurances();
            check_for_last_in_row_col_3x3();
        }
        return;
    }

    auto one_level_supposition()
    {
        auto progress = true;
        while (progress == true) {
            progress = false;
            for (auto row : range_el(0, 9))
                for (auto col : range_el(0, 9))
                    if (squares[row][col].len() > 1) {
                        Array<int> bad_x;
                        for (auto &&x : squares[row][col]) {
                            auto soduko_copy = copy();
                            try
                            {
                                soduko_copy.set_cell(row, col, x);
                                soduko_copy.check();
                            }
                            catch (const Error& e)
                            {
                                bad_x.append(x);
                            }
                        }

                        if (bad_x.empty()) {
                        }

                        else if (bad_x.len() < squares[row][col].len()) {
                            for (auto &&x : bad_x) {
                                cell_exclude(row, col, x);
                                check();
                            }
                            progress = true;
                        }
                        else
                            assert(false, u"bugger7"_S);
                    }
        }
    }
};

int main()
{
    for (auto x : range_el(0, 50)) {
        auto t = soduko(create_array({u"800000600"_S, u"040500100"_S, u"070090000"_S, u"030020007"_S, u"600008004"_S, u"500000090"_S, u"000030020"_S, u"001006050"_S, u"004000003"_S}));
        t.check();
        t.one_level_supposition();
        t.check();
        print(t);
    }
}
