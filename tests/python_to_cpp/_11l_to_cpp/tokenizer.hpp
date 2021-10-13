struct CodeBlock1
{
    CodeBlock1()
    {
        uR"(
===============================================================================================================
Ошибки:
---------------------------------------------------------------------------------------------------------------
Error: mixing tabs and spaces in indentation: `...`
---------------------------------------------------------------------------------------------------------------
В одной строке для отступа используется смесь пробелов и символов табуляции.
Выберите что-либо одно (желательно сразу для всего файла): либо пробелы для отступа, либо табуляцию.
Примечание: внутри строковых литералов, в комментариях, а также внутри строк кода можно смешивать пробелы и
табуляцию. Эта ошибка генерируется только при проверке отступов (отступ — последовательность символов пробелов
или табуляции от самого начала строки до первого символа отличного от пробела и табуляции).

---------------------------------------------------------------------------------------------------------------
Error: inconsistent indentations: ```...```
---------------------------------------------------------------------------------------------------------------
В текущей строке кода для отступа используются пробелы, а в предыдущей строке — табуляция (либо наоборот).
[[[
Сообщение было предназначено для несколько другой ошибки: для любых двух соседних строк, если взять отступ
одной из них, то другой отступ должен начинаться с него же {если отступ текущей строки отличается от отступа
предыдущей, то:
1. Когда отступ текущей строки начинается на отступ предыдущей строки, это INDENT.
2. Когда отступ предыдущей строки начинается на отступ текущей строки, это DEDENT.
}. Например:
if a:
SSTABif b:
SSTABTABi = 0
SSTABSi = 0
Последняя пара строк не удовлетворяет этому требованию, так как ни строка ‘SSTABTAB’ не начинается на строку
‘SSTABS’, ни ‘SSTABS’ не начинается на ‘SSTABTAB’.
Эта проверка имела бы смысл в случае разрешения смешения пробелов и табуляции для отступа в пределах одной
строки (а это разрешено в Python). Но я решил отказаться от этой идеи, а лучшего текста сообщения для этой
ошибки не придумал.
]]]
===============================================================================================================
)"_S;
    }
} code_block_1;

auto keywords = create_array({u"V"_S, u"C"_S, u"I"_S, u"E"_S, u"F"_S, u"L"_S, u"N"_S, u"R"_S, u"S"_S, u"T"_S, u"X"_S, u"П"_S, u"С"_S, u"Е"_S, u"И"_S, u"Ф"_S, u"Ц"_S, u"Н"_S, u"Р"_S, u"В"_S, u"Т"_S, u"Х"_S, u"var"_S, u"in"_S, u"if"_S, u"else"_S, u"fn"_S, u"loop"_S, u"null"_S, u"return"_S, u"switch"_S, u"type"_S, u"exception"_S, u"перем"_S, u"С"_S, u"если"_S, u"иначе"_S, u"фн"_S, u"цикл"_S, u"нуль"_S, u"вернуть"_S, u"выбрать"_S, u"тип"_S, u"исключение"_S});
Array<String> empty_list_of_str;
Array<Set<String>> binary_operators;

struct CodeBlock2
{
    CodeBlock2()
    {
        binary_operators.append(create_set(empty_list_of_str));
        binary_operators.append(create_set<String>({String(u"+"_S), u"-"_S, u"*"_S, u"/"_S, u"%"_S, u"^"_S, u"&"_S, u"|"_S, u"<"_S, u">"_S, u"="_S, u"?"_S}));
        binary_operators.append(create_set({u"<<"_S, u">>"_S, u"<="_S, u">="_S, u"=="_S, u"!="_S, u"+="_S, u"-="_S, u"*="_S, u"/="_S, u"%="_S, u"&="_S, u"|="_S, u"^="_S, u"->"_S, u".."_S, u".<"_S, u".+"_S, u"<."_S, u"I/"_S, u"Ц/"_S, u"C "_S, u"С "_S}));
        binary_operators.append(create_set({u"<<="_S, u">>="_S, u"‘’="_S, u"[+]"_S, u"[&]"_S, u"[|]"_S, u"(+)"_S, u"<.<"_S, u"I/="_S, u"Ц/="_S, u"in "_S, u"!C "_S, u"!С "_S}));
        binary_operators.append(create_set({u"[+]="_S, u"[&]="_S, u"[|]="_S, u"(+)="_S, u"!in "_S}));
    }
} code_block_2;

Array<Set<String>> unary_operators;

struct CodeBlock3
{
    CodeBlock3()
    {
        unary_operators.append(create_set(empty_list_of_str));
        unary_operators.append(create_set({String(u"!"_S)}));
        unary_operators.append(create_set({u"++"_S, u"--"_S}));
        unary_operators.append(create_set({u"(-)"_S}));
        unary_operators.append(create_set(empty_list_of_str));
    }
} code_block_3;

auto all_operators = Set<String>();
Array<Set<String>> operators;

struct CodeBlock4
{
    CodeBlock4()
    {
        for (int i = 0; i < 5; i++) {
            operators.append(binary_operators[i] | unary_operators[i]);
            all_operators |= binary_operators[i] | unary_operators[i];
        }
    }
} code_block_4;

auto first_char_of_operators = Set<Char>();

struct CodeBlock5
{
    CodeBlock5()
    {
        for (auto &&op : all_operators)
            first_char_of_operators.add(_get<0>(op));
        _get<1>(binary_operators).remove(u"^"_S);
        _get<2>(binary_operators).remove(u".."_S);
    }
} code_block_5;

class Error
{
public:
    String message;
    int pos;
    int end;

    template <typename T1, typename T2> Error(const T1 &message, const T2 &pos) :
        message(message),
        pos(pos),
        end(pos)
    {
    }
};

class Token
{
public:
    enum class Category {
        NAME,
        KEYWORD,
        CONSTANT,
        DELIMITER,
        OPERATOR,
        NUMERIC_LITERAL,
        STRING_LITERAL,
        STRING_CONCATENATOR,
        SCOPE_BEGIN,
        SCOPE_END,
        STATEMENT_SEPARATOR
    };

    int start;
    int end;
    Category category;

    template <typename T1, typename T2, typename T3> Token(const T1 &start, const T2 &end, const T3 &category) :
        start(start),
        end(end),
        category(category)
    {
    }

    auto __repr__()
    {
        return String(start);
    }

    template <typename T1> auto value(const T1 &source)
    {
        return source[range_el(start, end)];
    }

    template <typename T1> auto to_str(const T1 &source)
    {
        return u"Token("_S & String(category) & u", \""_S & (value(source)) & u"\")"_S;
    }
};

auto tokenize(const String &source, Array<Tuple<Char, int>>* const implied_scopes = nullptr, Array<int>* const line_continuations = nullptr, Array<ivec2>* const comments = nullptr)
{
    Array<Token> tokens;
    Array<Tuple<int, bool>> indentation_levels;
    Array<Tuple<Char, int>> nesting_elements;
    auto i = 0;
    auto begin_of_line = true;
    bool indentation_tabs;
    int prev_linestart;

    auto skip_multiline_comment = [&comments, &i, &source]()
    {
        auto comment_start = i;
        auto lbr = source[i + 1];
        auto rbr = [&](const auto &a){return a == u'‘' ? u'’'_C : a == u'(' ? u')'_C : a == u'{' ? u'}'_C : a == u'[' ? u']'_C : throw KeyError(a);}(lbr);
        i += 2;
        auto nesting_level = 1;
        while (true) {
            auto ch = source[i];
            i++;
            if (ch == lbr)
                nesting_level++;
            else if (ch == rbr) {
                nesting_level--;
                if (nesting_level == 0)
                    break;
            }
            if (i == source.len())
                throw Error(u"there is no corresponding opening parenthesis/bracket/brace/qoute for `"_S & lbr & u"`"_S, comment_start + 1);
        }
        if (comments != nullptr)
            comments->append(make_tuple(comment_start, i));
    };

    while (i < source.len()) {
        if (begin_of_line) {
            begin_of_line = false;
            auto linestart = i;
            auto tabs = false;
            auto spaces = false;
            while (i < source.len()) {
                if (source[i] == u' ')
                    spaces = true;
                else if (source[i] == u'\t')
                    tabs = true;
                else
                    break;
                i++;
            }
            if (i == source.len())
                break;

            auto ii = i;
            if (in(source[range_el(i, i + 2)], make_tuple(uR"(\‘)"_S, uR"(\()"_S, uR"(\{)"_S, uR"(\[)"_S))) {
                skip_multiline_comment();
                while (i < source.len() && in(source[i], u" \t"_S))
                    i++;
                if (i == source.len())
                    break;
            }

            if (in(source[i], u"\r\n"_S) || in(source[range_el(i, i + 2)], make_tuple(u"//"_S, uR"(\\)"_S)))
                continue;

            if (in(source[i], u"{}"_S))
                continue;

            if (tokens.len() && tokens.last().category == TYPE_RM_REF(tokens.last().category)::STRING_CONCATENATOR && in(source[i], u"\"'‘"_S)) {
                if (line_continuations != nullptr)
                    line_continuations->append(tokens.last().end);
                if (in(source[range_el(i, i + 2)], make_tuple(u"\"\""_S, u"‘’"_S)))
                    i += 2;
                continue;
            }

            if (tokens.len() && tokens.last().category == TYPE_RM_REF(tokens.last().category)::STRING_LITERAL && in(source[range_el(i, i + 2)], make_tuple(u"\"\""_S, u"‘’"_S))) {
                if (line_continuations != nullptr)
                    line_continuations->append(tokens.last().end);
                tokens.append(Token(i, i, Token::Category::STRING_CONCATENATOR));
                i += 2;
                continue;
            }

            if ((tokens.len() && tokens.last().category == TYPE_RM_REF(tokens.last().category)::OPERATOR && in(tokens.last().value(source), tokenizer::binary_operators[tokens.last().end - tokens.last().start]) && source[range_el(tokens.last().end - 4, tokens.last().end)] != u"-> &" && (tokens.last().value(source) != u'?' || source[tokens.last().start - 1] == u' '))) {
                if (line_continuations != nullptr)
                    line_continuations->append(tokens.last().end);
                continue;
            }

            if (((in(source[i], _get<1>(tokenizer::binary_operators)) || in(source[range_el(i, i + 2)], _get<2>(tokenizer::binary_operators)) || in(source[range_el(i, i + 3)], _get<3>(tokenizer::binary_operators)) || in(source[range_el(i, i + 4)], _get<4>(tokenizer::binary_operators))) && !(in(source[i], _get<1>(tokenizer::unary_operators)) || in(source[range_el(i, i + 2)], _get<2>(tokenizer::unary_operators)) || in(source[range_el(i, i + 3)], _get<3>(tokenizer::unary_operators))) && (!in(source[i], make_tuple(u"&"_S, u"-"_S)) || source[range_el(i + 1, i + 2)] == u' '))) {
                if (tokens.empty())
                    throw Error(u"source can not starts with a binary operator"_S, i);
                if (line_continuations != nullptr)
                    line_continuations->append(tokens.last().end);
                continue;
            }

            if (source[range_el(i, i + 2)] == uR"(\.)") {
                if (!tokens.empty()) {
                    i++;
                    if (line_continuations != nullptr)
                        line_continuations->append(tokens.last().end);
                }
                continue;
            }

            if (tabs && spaces) {
                auto next_line_pos = source.findi(u"\n"_S, i);
                throw Error(u"mixing tabs and spaces in indentation: `"_S & source[range_el(linestart, i)].replace(u" "_S, u"S"_S).replace(u"\t"_S, u"TAB"_S) & source[range_el(i, next_line_pos != -1 ? next_line_pos : source.len())] & u"`"_S, i);
            }

            auto indentation_level = ii - linestart;
            if (indentation_levels.len() && _get<0>(indentation_levels.last()) == -1) {
                assert(_get<1>(indentation_levels.last()));
                indentation_levels.last() = make_tuple(indentation_level, true);
                indentation_tabs = tabs;
            }
            else {
                auto prev_indentation_level = !indentation_levels.empty() ? _get<0>(indentation_levels.last()) : 0;

                if (indentation_level > 0 && prev_indentation_level > 0 && indentation_tabs != tabs) {
                    auto e = i + 1;
                    while (e < source.len() && !in(source[e], u"\r\n"_S))
                        e++;
                    throw Error(u"inconsistent indentations:\n```\n"_S & prev_indentation_level * (indentation_tabs ? u"TAB"_S : u"S"_S) & source[range_el(prev_linestart, linestart)] & (ii - linestart) * (tabs ? u"TAB"_S : u"S"_S) & source[range_el(ii, e)] & u"\n```"_S, ii);
                }
                prev_linestart = ii;

                if (indentation_level == prev_indentation_level) {
                    if (tokens.len() && tokens.last().category != TYPE_RM_REF(tokens.last().category)::SCOPE_END)
                        tokens.append(Token(linestart - 1, linestart, Token::Category::STATEMENT_SEPARATOR));
                }
                else if (indentation_level > prev_indentation_level) {
                    if (prev_indentation_level == 0)
                        indentation_tabs = tabs;
                    indentation_levels.append(make_tuple(indentation_level, false));
                    tokens.append(Token(linestart, ii, Token::Category::SCOPE_BEGIN));
                    if (implied_scopes != nullptr)
                        implied_scopes->append(make_tuple(u'{'_C, tokens.at_plus_len( - 2).end + (in(source[tokens.at_plus_len( - 2).end], u" \n"_S) ? 1 : 0)));
                }
                else
                    while (true) {
                        if (_get<1>(indentation_levels.last()))
                            throw Error(u"too much unindent, what is this unindent intended for?"_S, ii);
                        indentation_levels.pop();
                        tokens.append(Token(ii, ii, Token::Category::SCOPE_END));
                        if (implied_scopes != nullptr)
                            implied_scopes->append(make_tuple(u'}'_C, ii));
                        auto level = !indentation_levels.empty() ? _get<0>(indentation_levels.last()) : 0;
                        if (level == indentation_level)
                            break;
                        if (level < indentation_level)
                            throw Error(u"unindent does not match any outer indentation level"_S, ii);
                    }
            }
        }

        auto ch = source[i];

        if (in(ch, u" \t"_S))
            i++;
        else if (in(ch, u"\r\n"_S)) {
            i++;
            if (ch == u'\r' && source[range_el(i, i + 1)] == u'\n')
                i++;
            if (nesting_elements.empty() || !in(_get<0>(nesting_elements.last()), u"(["_S))
                begin_of_line = true;
        }
        else if ((ch == u'/' && source[range_el(i + 1, i + 2)] == u'/') || (ch == u'\\' && source[range_el(i + 1, i + 2)] == u'\\')) {
            auto comment_start = i;
            i += 2;
            while (i < source.len() && !in(source[i], u"\r\n"_S))
                i++;
            if (comments != nullptr)
                comments->append(make_tuple(comment_start, i));
        }
        else if (ch == u'\\' && in(source[range_el(i + 1, i + 2)], u"‘({["_S))
            skip_multiline_comment();
        else {
            auto is_hexadecimal_digit = [](const auto &ch)
            {
                return in(ch, range_ee(u'0'_C, u'9'_C)) || in(ch, range_ee(u'A'_C, u'F'_C)) || in(ch, range_ee(u'a'_C, u'f'_C)) || in(ch, u"абсдефАБСДЕФ"_S);
            };

            auto operator_s = u""_S;
            if (in(ch, tokenizer::first_char_of_operators))
                for (auto n : range_el(4, 0).step(-1))
                    if (in(source[range_el(i, i + n)], tokenizer::operators[n])) {
                        operator_s = source[range_el(i, i + n)];
                        if (operator_s == u'|' && in(source[range_el(i + 1, i + 2)], make_tuple(u"‘"_S, u"'"_S)))
                            operator_s = u""_S;
                        break;
                    }

            auto lexem_start = i;
            i++;
            Token::Category category;

            if (operator_s != u"") {
                i = lexem_start + operator_s.len();
                if (source[i - 1] == u' ')
                    i--;
                category = TYPE_RM_REF(category)::OPERATOR;
            }

            else if (ch.is_alpha() || in(ch, make_tuple(u"_"_S, u"@"_S))) {
                if (ch == u'@') {
                    while (i < source.len() && source[i] == u'@')
                        i++;
                    if (i < source.len() && source[i] == u'=')
                        i++;
                }
                while (i < source.len()) {
                    ch = source[i];
                    if (!(ch.is_alpha() || in(ch, u"_?:"_S) || in(ch, range_ee(u'0'_C, u'9'_C))))
                        break;
                    i++;
                }
                auto j = i - 1;
                while (j > lexem_start) {
                    if (source[j] == u':') {
                        i = j;
                        break;
                    }
                    j--;
                }

                if (source[range_el(i, i + 1)] == u'/' && in(source[range_el(i - 1, i)], u"IЦ"_S)) {
                    if (source[range_el(i - 2, i - 1)] == u' ')
                        category = TYPE_RM_REF(category)::OPERATOR;
                    else
                        throw Error(u"please clarify your intention by putting space character before or after `I`"_S, i - 1);
                }

                else if (source[range_el(i, i + 1)] == u'\'') {
                    i++;
                    if (in(source[range_el(i, i + 1)], make_tuple(u" "_S, u"\n"_S)))
                        category = TYPE_RM_REF(category)::NAME;
                    else if (in(source[range_el(i, i + 1)], make_tuple(u"‘"_S, u"'"_S))) {
                        i--;
                        category = TYPE_RM_REF(category)::NAME;
                    }
                    else {
                        while (i < source.len() && (is_hexadecimal_digit(source[i]) || source[i] == u'\''))
                            i++;
                        if (!(source[range_el(lexem_start + 4, lexem_start + 5)] == u'\'' || source[range_el(i - 3, i - 2)] == u'\'' || source[range_el(i - 2, i - 1)] == u'\''))
                            throw Error(u"digit separator in this hexadecimal number is located in the wrong place"_S, lexem_start);
                        category = TYPE_RM_REF(category)::NUMERIC_LITERAL;
                    }
                }

                else if (in(source[range_el(lexem_start, i)], tokenizer::keywords)) {
                    if (in(source[range_el(lexem_start, i)], make_tuple(u"V"_S, u"П"_S, u"var"_S, u"перем"_S))) {
                        category = TYPE_RM_REF(category)::NAME;
                        if (source[range_el(i, i + 1)] == u'&')
                            i++;
                    }
                    else if (in(source[range_el(lexem_start, i)], make_tuple(u"N"_S, u"Н"_S, u"null"_S, u"нуль"_S)))
                        category = TYPE_RM_REF(category)::CONSTANT;
                    else {
                        category = TYPE_RM_REF(category)::KEYWORD;
                        if (source[range_el(i, i + 1)] == u'.') {
                            i++;
                            while (i < source.len() && (source[i].is_alpha() || in(source[i], u"_."_S)))
                                i++;
                            if (in(source[range_el(lexem_start, i)], make_tuple(u"L.index"_S, u"Ц.индекс"_S, u"loop.index"_S, u"цикл.индекс"_S)))
                                category = TYPE_RM_REF(category)::NAME;
                        }
                    }
                }
                else
                    category = TYPE_RM_REF(category)::NAME;
            }

            else if (in(ch, range_ee(u'0'_C, u'9'_C)) || (ch == u'.' && in(source[range_el(i, i + 1)], range_ee(u'0'_C, u'9'_C)))) {
                if (in(ch, u"01"_S) && in(source[range_el(i, i + 1)], make_tuple(u"B"_S, u"В"_S)) && !(is_hexadecimal_digit(source[range_el(i + 1, i + 2)]) || source[range_el(i + 1, i + 2)] == u'\'')) {
                    i++;
                    category = TYPE_RM_REF(category)::CONSTANT;
                }
                else {
                    auto is_hex = false;
                    while (i < source.len() && is_hexadecimal_digit(source[i])) {
                        if (!(in(source[i], range_ee(u'0'_C, u'9'_C)))) {
                            if (in(source[i], u"eE"_S) && in(source[range_el(i + 1, i + 2)], make_tuple(u"-"_S, u"+"_S)))
                                break;
                            is_hex = true;
                        }
                        i++;
                    }

                    auto next_digit_separator = 0;
                    auto is_oct_or_bin = false;
                    if (i < source.len() && source[i] == u'\'') {
                        if (in(i - lexem_start, make_tuple(2, 1))) {
                            auto j = i + 1;
                            while (j < source.len() && is_hexadecimal_digit(source[j])) {
                                if (!(in(source[j], range_ee(u'0'_C, u'9'_C))))
                                    is_hex = true;
                                j++;
                            }
                            next_digit_separator = j - 1 - i;
                        }
                        else if (i - lexem_start == 4) {
                            auto j = i + 1;
                            while (j < source.len() && ((is_hexadecimal_digit(source[j]) && !(in(source[j], u"bд"_S))) || source[j] == u'\''))
                                j++;
                            if (j < source.len() && in(source[j], u"oоbд"_S))
                                is_oct_or_bin = true;
                        }
                    }

                    if (i < source.len() && source[i] == u'\'' && ((i - lexem_start == 4 && !is_oct_or_bin) || (in(i - lexem_start, make_tuple(2, 1)) && (next_digit_separator != 3 || is_hex)))) {
                        if (i - lexem_start == 2)
                            while (true) {
                                i++;
                                if (i + 2 > source.len() || !is_hexadecimal_digit(source[i]) || !is_hexadecimal_digit(source[i + 1]))
                                    throw Error(u"wrong short hexadecimal number"_S, lexem_start);
                                i += 2;
                                if (i < source.len() && is_hexadecimal_digit(source[i]))
                                    throw Error(u"expected end of short hexadecimal number"_S, i);
                                if (source[range_el(i, i + 1)] != u'\'')
                                    break;
                            }
                        else if (i - lexem_start == 1) {
                            i++;
                            if (i + 1 > source.len() || !is_hexadecimal_digit(source[i]))
                                throw Error(u"wrong ultrashort hexadecimal number"_S, lexem_start);
                            i++;
                            if (i < source.len() && is_hexadecimal_digit(source[i]))
                                throw Error(u"expected end of ultrashort hexadecimal number"_S, i);
                        }
                        else {
                            i++;
                            while (i < source.len() && is_hexadecimal_digit(source[i])) {
                                i++;
                                if (mod((i - lexem_start), 5) == 4 && i < source.len()) {
                                    if (source[i] != u'\'') {
                                        if (!is_hexadecimal_digit(source[i]))
                                            break;
                                        throw Error(u"here should be a digit separator in hexadecimal number"_S, i);
                                    }
                                    i++;
                                }
                            }
                            if (i < source.len() && source[i] == u'\'')
                                throw Error(u"digit separator in hexadecimal number is located in the wrong place"_S, i);
                            if (mod((i - lexem_start), 5) != 4)
                                throw Error(u"after this digit separator there should be 4 digits in hexadecimal number"_S, source.rfindi(u"'"_S, 0, i));
                        }
                    }
                    else {
                        while (i < source.len() && (in(source[i], range_ee(u'0'_C, u'9'_C)) || in(source[i], u"'.eE"_S))) {
                            if (in(source[range_el(i, i + 2)], make_tuple(u".."_S, u".<"_S, u".+"_S)))
                                break;
                            if (in(source[i], u"eE"_S)) {
                                if (in(source[range_el(i + 1, i + 2)], u"-+"_S))
                                    i++;
                            }
                            i++;
                        }
                        if (in(source[range_el(i, i + 1)], make_tuple(u"o"_S, u"о"_S, u"b"_S, u"д"_S, u"s"_S, u"i"_S)))
                            i++;
                        else if (in(u'\''_C, source[range_el(lexem_start, i)]) && !(in(u'.'_C, source[range_el(lexem_start, i)]))) {
                            auto number = source[range_el(lexem_start, i)].replace(u"'"_S, u""_S);
                            auto number_with_separators = u""_S;
                            auto j = number.len();
                            while (j > 3) {
                                number_with_separators = u"'"_S & number[range_el(j - 3, j)] & number_with_separators;
                                j -= 3;
                            }
                            number_with_separators = number[range_el(0, j)] & number_with_separators;
                            if (source[range_el(lexem_start, i)] != number_with_separators)
                                throw Error(u"digit separator in this number is located in the wrong place (should be: "_S & number_with_separators & u")"_S, lexem_start);
                        }
                    }
                    category = TYPE_RM_REF(category)::NUMERIC_LITERAL;
                }
            }

            else if (ch == u'\'' && source[range_el(i, i + 1)] == u',') {
                i++;
                category = TYPE_RM_REF(category)::DELIMITER;
            }

            else if (ch == u'"') {
                if (source[i] == u'"' && tokens.last().category == TYPE_RM_REF(tokens.last().category)::STRING_CONCATENATOR && tokens.at_plus_len( - 2).category == TYPE_RM_REF(tokens.at_plus_len( - 2).category)::STRING_LITERAL && _get<0>(tokens.at_plus_len( - 2).value(source)) == u'‘') {
                    i++;
                    continue;
                }
                auto startqpos = i - 1;
                if (tokens.len() && tokens.last().end == startqpos && ((tokens.last().category == TYPE_RM_REF(tokens.last().category)::NAME && tokens.last().value(source).last() != u'\'') || in(tokens.last().value(source), make_tuple(u")"_S, u"]"_S))))
                    tokens.append(Token(lexem_start, lexem_start, Token::Category::STRING_CONCATENATOR));
                while (true) {
                    if (i == source.len())
                        throw Error(u"unclosed string literal"_S, startqpos);
                    ch = source[i];
                    i++;
                    if (ch == u'\\') {
                        if (i == source.len())
                            continue;
                        i++;
                    }
                    else if (ch == u'"')
                        break;
                }
                if (source[range_el(i, i + 1)].is_alpha() || in(source[range_el(i, i + 1)], make_tuple(u"_"_S, u"@"_S, u":"_S, u"‘"_S, u"("_S))) {
                    tokens.append(Token(lexem_start, i, Token::Category::STRING_LITERAL));
                    tokens.append(Token(i, i, Token::Category::STRING_CONCATENATOR));
                    continue;
                }
                category = TYPE_RM_REF(category)::STRING_LITERAL;
            }

            else if (in(ch, u"‘'"_S) || (ch == u'|' && in(source[range_el(i, i + 1)], make_tuple(u"‘"_S, u"'"_S)))) {
                if (source[i] == u'’' && tokens.last().category == TYPE_RM_REF(tokens.last().category)::STRING_CONCATENATOR && tokens.at_plus_len( - 2).category == TYPE_RM_REF(tokens.at_plus_len( - 2).category)::STRING_LITERAL && _get<0>(tokens.at_plus_len( - 2).value(source)) == u'"') {
                    i++;
                    continue;
                }
                if (tokens.len() && tokens.last().end == i - 1 && ((tokens.last().category == TYPE_RM_REF(tokens.last().category)::NAME && tokens.last().value(source).last() != u'\'') || in(tokens.last().value(source), make_tuple(u")"_S, u"]"_S)))) {
                    tokens.append(Token(lexem_start, lexem_start, Token::Category::STRING_CONCATENATOR));
                    if (source[i] == u'’') {
                        i++;
                        continue;
                    }
                }
                if (ch != u'|')
                    i--;
                while (i < source.len() && source[i] == u'\'')
                    i++;
                if (source[range_el(i, i + 1)] != u'‘')
                    throw Error(u"expected left single quotation mark"_S, i);
                auto startqpos = i;
                i++;
                auto nesting_level = 1;
                while (true) {
                    if (i == source.len())
                        throw Error(u"unpaired left single quotation mark"_S, startqpos);
                    ch = source[i];
                    i++;
                    if (ch == u'‘')
                        nesting_level++;
                    else if (ch == u'’') {
                        nesting_level--;
                        if (nesting_level == 0)
                            break;
                    }
                }
                while (i < source.len() && source[i] == u'\'')
                    i++;
                if (source[range_el(i, i + 1)].is_alpha() || in(source[range_el(i, i + 1)], make_tuple(u"_"_S, u"@"_S, u":"_S, u"\""_S, u"("_S))) {
                    tokens.append(Token(lexem_start, i, Token::Category::STRING_LITERAL));
                    tokens.append(Token(i, i, Token::Category::STRING_CONCATENATOR));
                    continue;
                }
                category = TYPE_RM_REF(category)::STRING_LITERAL;
            }

            else if (ch == u'{') {
                indentation_levels.append(make_tuple(-1, true));
                nesting_elements.append(make_tuple(u'{'_C, lexem_start));
                category = TYPE_RM_REF(category)::SCOPE_BEGIN;
            }
            else if (ch == u'}') {
                if (nesting_elements.empty() || _get<0>(nesting_elements.last()) != u'{')
                    throw Error(u"there is no corresponding opening brace for `}`"_S, lexem_start);
                nesting_elements.pop();
                while (_get<1>(indentation_levels.last()) != true) {
                    tokens.append(Token(lexem_start, lexem_start, Token::Category::SCOPE_END));
                    if (implied_scopes != nullptr)
                        implied_scopes->append(make_tuple(u'}'_C, lexem_start));
                    indentation_levels.pop();
                }
                assert(_get<1>(indentation_levels.pop()) == true);
                category = TYPE_RM_REF(category)::SCOPE_END;
            }
            else if (ch == u';')
                category = TYPE_RM_REF(category)::STATEMENT_SEPARATOR;

            else if (in(ch, make_tuple(u","_S, u"."_S, u":"_S)))
                category = TYPE_RM_REF(category)::DELIMITER;

            else if (in(ch, u"(["_S)) {
                if (source[range_el(lexem_start, lexem_start + 3)] == u"(.)") {
                    i += 2;
                    category = TYPE_RM_REF(category)::NAME;
                }
                else {
                    nesting_elements.append(make_tuple(ch, lexem_start));
                    category = TYPE_RM_REF(category)::DELIMITER;
                }
            }
            else if (in(ch, u"])"_S)) {
                if (nesting_elements.empty() || _get<0>(nesting_elements.last()) != ([&](const auto &a){return a == u']' ? u'['_C : a == u')' ? u'('_C : throw KeyError(a);}(ch)))
                    throw Error(u"there is no corresponding opening parenthesis/bracket for `"_S & ch & u"`"_S, lexem_start);
                nesting_elements.pop();
                category = TYPE_RM_REF(category)::DELIMITER;
            }

            else
                throw Error(u"unexpected character `"_S & ch & u"`"_S, lexem_start);

            tokens.append(Token(lexem_start, i, category));
        }
    }

    if (!nesting_elements.empty())
        throw Error(u"there is no corresponding closing parenthesis/bracket/brace for `"_S & _get<0>(nesting_elements.last()) & u"`"_S, _get<1>(nesting_elements.last()));

    while (!indentation_levels.empty()) {
        assert(_get<1>(indentation_levels.last()) != true);
        tokens.append(Token(i, i, Token::Category::SCOPE_END));
        if (implied_scopes != nullptr)
            implied_scopes->append(make_tuple(u'}'_C, source.last() == u'\n' ? i - 1 : i));
        indentation_levels.pop();
    }

    return tokens;
}
