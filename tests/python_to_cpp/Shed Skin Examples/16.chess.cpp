#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto iNone = -999;
auto iTrue = 1;
auto iFalse = 0;

auto setup = create_array({4, 2, 3, 5, 6, 3, 2, 4, iNone, iNone}) + create_array({iTrue}) * 4 + create_array({iNone, iNone}) + create_array({1}) * 8 + create_array({iNone, iNone, iTrue, iNone, iNone, iNone, iNone, iNone}) + (create_array({0}) * 8 + create_array({iNone}) * 8) * 4 + create_array({-1}) * 8 + create_array({iNone}) * 8 + create_array({-4, -2, -3, -5, -6, -3, -2, -4}) + create_array({iNone}) * 40;

auto squares = range_el(0, 128).filter([](const auto &i){return !(i & 8);});
auto knightMoves = create_array({-33, -31, -18, -14, 14, 18, 31, 33});
auto bishopLines = create_array({create_array(range_el(17, 120).step(17)), create_array(range_el(-17, -120).step(-17)), create_array(range_el(15, 106).step(15)), create_array(range_el(-15, -106).step(-15))});
auto rookLines = create_array({create_array(range_ee(1, 7)), create_array(range_el(-1, -8).step(-1)), create_array(range_el(16, 128).step(16)), create_array(range_el(-16, -128).step(-16))});
auto queenLines = bishopLines + rookLines;
auto kingMoves = make_tuple(-17, -16, -15, -1, 1, 15, 16, 17);

Array<int> empty_list;
auto linePieces = create_array({create_array({empty_list}), create_array({empty_list}), create_array({empty_list}), bishopLines, rookLines, queenLines, create_array({empty_list}), create_array({empty_list}), queenLines, rookLines, bishopLines, create_array({empty_list}), create_array({empty_list})});

auto clearCastlingOpportunities = create_array({empty_list}) * 0x80;

struct CodeBlock1
{
    CodeBlock1()
    {
        for (auto &&[i, v] : make_tuple(make_tuple(0x00, create_array({10})), make_tuple(0x04, create_array({10, 11})), make_tuple(0x07, create_array({11})), make_tuple(0x70, create_array({12})), make_tuple(0x74, create_array({12, 13})), make_tuple(0x77, create_array({13}))))
            clearCastlingOpportunities.set(i, v);
    }
} code_block_1;

auto pieces = u".pnbrqkKQRBNP"_S;

template <typename T1, typename T2> auto listGet(const T1 &l, const T2 &i)
{
    return i >= 0 ? l[i] : l.at_plus_len( - (-i));
}

template <typename T1> auto evaluate(const T1 &board)
{
    auto evals = create_array({0, 100, 300, 330, 510, 950, 100000, -100000, -950, -510, -330, -300, -100});
    return sum_map(::squares, [&board, &evals](const auto &i){return listGet(evals, board[i]);});
}

template <typename T1> auto printBoard(const T1 &board)
{
    for (auto i : range_el(7, -1).step(-1)) {
        for (int j = 0; j < 8; j++) {
            auto ix = i * 16 + j;
            print(::pieces[board[ix]], u" "_S);
        }
        print();
    }
}

template <typename T2> auto mov(Array<int> &board, const T2 &mv)
{
    auto ix = (mv >> 8) & 0xFF;
    board.set(mv & 0xFF, board[ix]);
    board.set(ix, 0);
    for (auto &&i : ::clearCastlingOpportunities[ix])
        board.set(i, ::iFalse);

    _set<26>(board, to_int(!_get<26>(board)));
    if ((mv & 0x7FFF'0000) == 0)
        return;
    if ((mv & 0x0100'0000))
        _set<27>(board, mv & 7);
    else
        _set<27>(board, ::iNone);
    if ((mv & 0x0400'0000))
        switch (mv & 0xFF)
        {
        case 0x02:
            _set<0x00>(board, 0);
            _set<0x03>(board, 4);
            break;
        case 0x06:
            _set<0x07>(board, 0);
            _set<0x05>(board, 4);
            break;
        case 0x72:
            _set<0x70>(board, 0);
            _set<0x73>(board, -4);
            break;
        case 0x76:
            _set<0x77>(board, 0);
            _set<0x75>(board, -4);
            break;
        default:
            throw u"faulty castling"_S;
            break;
        }
    if (mv & 0x0800'0000) {
        if (_get<26>(board))
            board.set(mv & 0x07 + 64, 0);
        else
            board.set(mv & 0x07 + 48, 0);
    }
    if (mv & 0x1000'0000) {
        auto a = (mv & 0x00FF'0000) >> 16;
        if ((a >= 0x80))
            a = a - 0x01'00;
        board.set(mv & 0xFF, a);
    }
}

template <typename T1> auto toString(const T1 &move)
{
    auto fr = (move >> 8) & 0xFF;
    auto to = move & 0xFF;
    auto letters = u"abcdefgh"_S;
    auto numbers = u"12345678"_S;
    auto mid = u"-"_S;
    if ((move & 0x0400'0000)) {
        if ((move & 0x07) == 0x02)
            return u"O-O-O"_S;
        else
            return u"O-O"_S;
    }
    if (move & 0x0200'0000)
        mid = u"x"_S;
    auto retval = letters[fr & 7] & numbers[fr >> 4] & mid & letters[to & 7] & numbers[to >> 4];
    return retval;
}

template <typename T2> auto moveStr(Array<int> &board, const T2 &strMove)
{
    auto moves = pseudoLegalMoves(board);
    for (auto &&m : moves)
        if (strMove == toString(m)) {
            mov(board, m);
            return;
        }
    for (auto &&m : moves)
        print(toString(m));
    throw u"no move found"_S;
}

template <typename T1, typename T2, typename T3, typename T4> auto rowAttack(const T1 &board, const T2 &attackers, const T3 &ix, const T4 &dir)
{
    auto own = _get<0>(attackers);
    for (auto &&k : dir.map([&ix](const auto &i){return i + ix;})) {
        if (k & 0x88)
            return false;
        if (board[k])
            return (board[k] * own < 0) && in(board[k], attackers);
    }
}

template <typename T1, typename T2, typename T3> auto nonpawnAttacks(const T1 &board, const T2 &ix, const T3 &color)
{
    return (max_map(::knightMoves, [&board, &color, &ix](const auto &i){return board[ix + i] == color * 2;}) || max_map(::bishopLines, [&board, &color, &ix](const auto &bishopLine){return rowAttack(board, make_tuple(color * 3, color * 5), ix, bishopLine);}) || max_map(::rookLines, [&board, &color, &ix](const auto &rookLine){return rowAttack(board, make_tuple(color * 4, color * 5), ix, rookLine);}));
}

auto nonpawnBlackAttacks = [](const auto &board, const auto &ix){return nonpawnAttacks(board, ix, -1);};
auto nonpawnWhiteAttacks = [](const auto &board, const auto &ix){return nonpawnAttacks(board, ix, 1);};

template <typename T1> auto pseudoLegalMovesWhite(const T1 &board)
{
    auto retval = pseudoLegalCapturesWhite(board);
    for (auto &&sq : ::squares) {
        auto b = board[sq];
        if (b >= 1) {
            if (b == 1 && !(sq + 16 & 0x88) && board[sq + 16] == 0) {
                if (sq >= 16 && sq < 32 && board[sq + 32] == 0)
                    retval.append(sq * 0x01'01 + 32);
                retval.append(sq * 0x01'01 + 16);
            }
            else if (b == 2) {
                for (auto &&k : ::knightMoves)
                    if (listGet(board, k + sq) == 0)
                        retval.append(sq * 0x01'01 + k);
            }
            else if (b == 3 || b == 5)
                for (auto &&line : ::bishopLines)
                    for (auto &&k : line) {
                        if ((k + sq & 0x88) || board[k + sq] != 0)
                            break;
                        retval.append(sq * 0x01'01 + k);
                    }
            if (b == 4 || b == 5)
                for (auto &&line : ::rookLines)
                    for (auto &&k : line) {
                        if ((k + sq & 0x88) || board[k + sq] != 0)
                            break;
                        retval.append(sq * 0x01'01 + k);
                    }
            else if (b == 6) {
                for (auto &&k : ::kingMoves)
                    if (!(k + sq & 0x88) && board[k + sq] == 0)
                        retval.append(sq * 0x01'01 + k);
            }
        }
    }
    if ((_get<10>(board) && _get<1>(board) == 0 && _get<2>(board) == 0 && _get<3>(board) == 0 && !(in(-1, board[range_el(17, 22)])) && !nonpawnBlackAttacks(board, 2) && !nonpawnBlackAttacks(board, 3) && !nonpawnBlackAttacks(board, 4)))
        retval.append(0x0400'0000 + 4 * 0x01'01 - 2);
    if ((_get<11>(board) && _get<5>(board) == 0 && _get<6>(board) == 0 && !(in(-1, board[range_el(19, 24)])) && !nonpawnBlackAttacks(board, 4) && !nonpawnBlackAttacks(board, 5) && !nonpawnBlackAttacks(board, 6)))
        retval.append(0x0400'0000 + 4 * 0x01'01 + 2);
    return retval;
}

template <typename T1> auto pseudoLegalMovesBlack(const T1 &board)
{
    auto retval = pseudoLegalCapturesBlack(board);
    for (auto &&sq : ::squares) {
        auto b = board[sq];
        if (b < 0) {
            if (b == -1 && !(sq + 16 & 0x88) && board[sq - 16] == 0) {
                if (sq >= 96 && sq < 112 && board[sq - 32] == 0)
                    retval.append(sq * 0x01'01 - 32);
                retval.append(sq * 0x01'01 - 16);
            }
            else if (b == -2) {
                for (auto &&k : ::knightMoves)
                    if (board[k + sq] == 0)
                        retval.append(sq * 0x01'01 + k);
            }
            else if (b == -3 || b == -5)
                for (auto &&line : ::bishopLines)
                    for (auto &&k : line) {
                        if ((k + sq & 0x88) || board[k + sq] != 0)
                            break;
                        retval.append(sq * 0x01'01 + k);
                    }

            if (b == -4 || b == -5)
                for (auto &&line : ::rookLines)
                    for (auto &&k : line) {
                        if ((k + sq & 0x88) || board[k + sq] != 0)
                            break;
                        retval.append(sq * 0x01'01 + k);
                    }
            else if (b == -6) {
                for (auto &&k : ::kingMoves)
                    if (!(k + sq & 0x88) && board[k + sq] == 0)
                        retval.append(sq * 0x01'01 + k);
            }
        }
    }
    if ((_get<12>(board) && _get<0x71>(board) == 0 && _get<0x72>(board) == 0 && _get<0x73>(board) == 0 && !(in(1, board[range_el(0x61, 0x65)])) && !nonpawnWhiteAttacks(board, 0x72) && !nonpawnWhiteAttacks(board, 0x73) && !nonpawnWhiteAttacks(board, 0x74)))
        retval.append(0x0400'0000 + 0x74 * 0x01'01 - 2);
    if ((_get<11>(board) && _get<0x75>(board) == 0 && _get<0x76>(board) == 0 && !(in(-1, board[range_el(0x63, 0x68)])) && !nonpawnWhiteAttacks(board, 0x74) && !nonpawnWhiteAttacks(board, 0x75) && !nonpawnWhiteAttacks(board, 0x76)))
        retval.append(0x0400'0000 + 0x74 * 0x01'01 + 2);
    return retval;
}

template <typename T1> auto pseudoLegalMoves(const T1 &board)
{
    if (_get<26>(board))
        return pseudoLegalMovesWhite(board);
    else
        return pseudoLegalMovesBlack(board);
}

template <typename T1> auto pseudoLegalCapturesWhite(const T1 &board)
{
    Array<int> retval;
    for (auto &&sq : ::squares) {
        auto b = board[sq];
        if (b >= 1) {
            if (b == 1) {
                if (!(sq + 17 & 0x88) && board[sq + 17] < 0)
                    retval.append(0x0200'0000 + sq * 0x01'01 + 17);
                if (!(sq + 15 & 0x88) && board[sq + 15] < 0)
                    retval.append(0x0200'0000 + sq * 0x01'01 + 15);
                if (sq >= 64 && sq < 72 && abs((sq & 7) - _get<27>(board)) == 1)
                    retval.append(0x0200'0000 + sq * 0x01'00 + (sq & 0x70) + 16 + _get<27>(board));
            }
            else if (b == 2) {
                for (auto &&k : ::knightMoves)
                    if (!(sq + k & 0x88) && board[k + sq] < 0)
                        retval.append(0x0200'0000 + sq * 0x01'01 + k);
            }
            else if (b == 6) {
                for (auto &&k : ::kingMoves)
                    if (!(k + sq & 0x88) && board[k + sq] < 0)
                        retval.append(0x0200'0000 + sq * 0x01'01 + k);
            }
            else
                for (auto &&line : ::linePieces[b])
                    for (auto &&k : line) {
                        if ((sq + k & 0x88) || board[k + sq] >= 1)
                            break;
                        if (board[k + sq] < 0) {
                            retval.append(0x0200'0000 + sq * 0x01'01 + k);
                            break;
                        }
                    }
        }
    }
    return retval;
}

template <typename T1> auto pseudoLegalCapturesBlack(const T1 &board)
{
    Array<int> retval;
    for (auto &&sq : ::squares) {
        auto b = board[sq];
        if (b < 0) {
            if (b == -1) {
                if (listGet(board, sq - 17) >= 1)
                    retval.append(0x0200'0000 + sq * 0x01'01 - 17);
                if (listGet(board, sq - 15) >= 1)
                    retval.append(0x0200'0000 + sq * 0x01'01 - 15);
                if (sq >= 48 && sq < 56 && abs((sq & 7) - _get<27>(board)) == 1)
                    retval.append(0x0A00'0000 + sq * 0x01'00 + (sq & 0x70) - 16 + _get<27>(board));
            }
            else if (b == -2) {
                for (auto &&k : ::knightMoves)
                    if (!(sq + k & 0x88) && board[k + sq] >= 1)
                        retval.append(0x0200'0000 + sq * 0x01'01 + k);
            }
            else if (b == -3)
                for (auto &&line : ::bishopLines)
                    for (auto &&k : line) {
                        if (listGet(board, k + sq) < 0)
                            break;
                        if (listGet(board, k + sq) >= 1) {
                            retval.append(0x0200'0000 + sq * 0x01'01 + k);
                            break;
                        }
                    }
            else if (b == -4)
                for (auto &&line : ::rookLines)
                    for (auto &&k : line) {
                        if (listGet(board, k + sq) < 0)
                            break;
                        if (listGet(board, k + sq) >= 1) {
                            retval.append(0x0200'0000 + sq * 0x01'01 + k);
                            break;
                        }
                    }
            else if (b == -5)
                for (auto &&line : ::queenLines)
                    for (auto &&k : line) {
                        if (listGet(board, k + sq) < 0)
                            break;
                        if (listGet(board, k + sq) >= 1) {
                            retval.append(0x0200'0000 + sq * 0x01'01 + k);
                            break;
                        }
                    }
            else if (b == -6) {
                for (auto &&k : ::kingMoves)
                    if (board[k + sq] >= 1)
                        retval.append(0x0200'0000 + sq * 0x01'01 + k);
            }
        }
    }
    return retval;
}

template <typename T1> auto pseudoLegalCaptures(const T1 &board)
{
    if (_get<26>(board))
        return pseudoLegalCapturesWhite(board);
    else
        return pseudoLegalCapturesBlack(board);
}

template <typename T1> auto legalMoves(const T1 &board)
{
    auto allMoves = pseudoLegalMoves(board);
    Array<int> retval;
    auto kingVal = 6;
    if (_get<26>(board))
        kingVal = -kingVal;
    for (auto &&mv : allMoves) {
        auto board2 = copy(board);
        mov(board2, mv);
        if (pseudoLegalCaptures(board2).filter([&board2, &kingVal](const auto &i){return board2[i & 0xFF] == kingVal;}).empty())
            retval.append(mv);
    }
    return retval;
}

template <typename T1, typename T2, typename T3, typename T4> auto alphaBetaQui(const T1 &board, T2 alpha, const T3 &beta, const T4 &n)
{
    auto e = evaluate(board);
    if (!_get<26>(board))
        e = -e;
    if (e >= beta)
        return make_tuple(beta, ::iNone);
    if ((e > alpha))
        alpha = e;
    auto bestMove = ::iNone;
    if (n >= -4)
        for (auto &&mv : pseudoLegalCaptures(board)) {
            auto newboard = copy(board);
            mov(newboard, mv);
            auto value = alphaBetaQui(newboard, -beta, -alpha, n - 1);
            value = make_tuple(-_get<0>(value), _get<1>(value));
            if (_get<0>(value) >= beta)
                return make_tuple(beta, mv);
            if ((_get<0>(value) > alpha)) {
                alpha = _get<0>(value);
                bestMove = mv;
            }
        }
    return make_tuple(alpha, bestMove);
}

template <typename T1, typename T2, typename T3, typename T4> auto alphaBeta(const T1 &board, T2 alpha, const T3 &beta, const T4 &n)
{
    if (n == 0)
        return alphaBetaQui(board, alpha, beta, n);
    auto bestMove = ::iNone;

    for (auto &&mv : legalMoves(board)) {
        auto newboard = copy(board);
        mov(newboard, mv);
        auto value = alphaBeta(newboard, -beta, -alpha, n - 1);
        value = make_tuple(-_get<0>(value), _get<1>(value));
        if (_get<0>(value) >= beta)
            return make_tuple(beta, mv);
        if ((_get<0>(value) > alpha)) {
            alpha = _get<0>(value);
            bestMove = mv;
        }
    }
    return make_tuple(alpha, bestMove);
}

auto speedTest()
{
    auto board = ::setup;
    moveStr(board, u"c2-c4"_S);
    moveStr(board, u"e7-e5"_S);
    moveStr(board, u"d2-d4"_S);

    auto res = alphaBeta(board, -99999999, 99999999, 4);
    print(res);
    moveStr(board, u"d7-d6"_S);
    res = alphaBeta(board, -99999999, 99999999, 4);
    print(res);
}

int main()
{
    speedTest();
}
