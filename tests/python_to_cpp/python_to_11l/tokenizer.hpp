auto keywords = create_array({u"False"_S, u"await"_S, u"else"_S, u"import"_S, u"pass"_S, u"None"_S, u"break"_S, u"except"_S, u"in"_S, u"raise"_S, u"True"_S, u"class"_S, u"finally"_S, u"is"_S, u"return"_S, u"and"_S, u"continue"_S, u"for"_S, u"lambda"_S, u"try"_S, u"as"_S, u"def"_S, u"from"_S, u"nonlocal"_S, u"while"_S, u"assert"_S, u"del"_S, u"global"_S, u"not"_S, u"with"_S, u"async"_S, u"elif"_S, u"if"_S, u"or"_S, u"yield"_S});
auto operators = create_array({u"+"_S, u"-"_S, u"*"_S, u"**"_S, u"/"_S, u"//"_S, u"%"_S, u"@"_S, u"<<"_S, u">>"_S, u"&"_S, u"|"_S, u"^"_S, u"~"_S, u"<"_S, u">"_S, u"<="_S, u">="_S, u"=="_S, u"!="_S});
auto delimiters = create_array({u"("_S, u")"_S, u"["_S, u"]"_S, u"{"_S, u"}"_S, u","_S, u":"_S, u"."_S, u";"_S, u"@"_S, u"="_S, u"->"_S, u"+="_S, u"-="_S, u"*="_S, u"/="_S, u"//="_S, u"%="_S, u"@="_S, u"&="_S, u"|="_S, u"^="_S, u">>="_S, u"<<="_S, u"**="_S});
auto operators_and_delimiters = sorted(operators + delimiters, [](const auto &x){return x.len();}, true);

class Error
{
public:
    String message;
    int pos;
    int end;

    template <typename T1, typename T2> Error(const T1 &message, const T2 &pos)
    {
        this->message = message;
        this->pos = pos;
        end = pos;
    }
};

class Token
{
public:
    enum class Category {
        NAME, 
        KEYWORD, 
        CONSTANT, 
        OPERATOR_OR_DELIMITER, 
        NUMERIC_LITERAL, 
        STRING_LITERAL, 
        INDENT, 
        DEDENT, 
        STATEMENT_SEPARATOR
    };
    int start;
    int end;
    Category category;

    template <typename T1, typename T2, typename T3> Token(const T1 &start, const T2 &end, const T3 &category)
    {
        this->start = start;
        this->end = end;
        this->category = category;
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
        return u"Token("_S + String(category) + u", \""_S + (value(source)) + u"\")"_S;
    }
};

template <typename T1> auto tokenize(const T1 &source, Array<int>* const newline_chars = nullptr, Array<Tuple<int, int>>* const comments = nullptr)
{
    Array<Token> tokens;
    Array<int> indentation_levels;
    Array<Tuple<Char, int>> nesting_elements;
    auto begin_of_line = true;
    auto expected_an_indented_block = false;
    auto i = 0;

    while (i < source.len()) {
        if (begin_of_line) {
            begin_of_line = false;
            auto linestart = i;
            auto indentation_level = 0;
            while (i < source.len()) {
                if (source[i] == u' ')
                    indentation_level++;
                else if (source[i] == u'\t')
                    indentation_level += 8;
                else
                    break;
                i++;
            }
            if (i == source.len())
                break;
            if (in(source[i], u"\r\n#"_S))
                continue;
            auto prev_indentation_level = !indentation_levels.empty() ? indentation_levels.last() : 0;

            if (expected_an_indented_block) {
                if (!(indentation_level > prev_indentation_level))
                    throw Error(u"expected an indented block"_S, i);
            }

            if (indentation_level == prev_indentation_level) {
                if (!tokens.empty())
                    tokens.append(Token(linestart - 1, linestart, Token::Category::STATEMENT_SEPARATOR));
            }
            else if (indentation_level > prev_indentation_level) {
                if (!expected_an_indented_block)
                    throw Error(u"unexpected indent"_S, i);
                expected_an_indented_block = false;
                indentation_levels.append(indentation_level);
                tokens.append(Token(linestart, i, Token::Category::INDENT));
            }
            else
                while (true) {
                    indentation_levels.pop();
                    tokens.append(Token(i, i, Token::Category::DEDENT));
                    auto level = !indentation_levels.empty() ? indentation_levels.last() : 0;
                    if (level == indentation_level)
                        break;
                    if (level < indentation_level)
                        throw Error(u"unindent does not match any outer indentation level"_S, i);
                }
        }
        auto ch = source[i];
        if (in(ch, u" \t"_S))
            i++;
        else if (in(ch, u"\r\n"_S)) {
            if (newline_chars != nullptr)
                newline_chars->append(i);
            i++;
            if (ch == u'\r' && source[range_el(i, i + 1)] == u'\n')
                i++;
            if (nesting_elements.empty())
                begin_of_line = true;
        }
        else if (ch == u'#') {
            auto comment_start = i;
            i++;
            while (i < source.len() && !in(source[i], u"\r\n"_S))
                i++;
            if (comments != nullptr)
                comments->append(make_tuple(comment_start, i));
        }
        else {
            expected_an_indented_block = ch == u':';
            auto operator_or_delimiter = u""_S;
            for (auto op : tokenizer::operators_and_delimiters)
                if (source[range_el(i, i + op.len())] == op) {
                    operator_or_delimiter = op;
                    break;
                }
            auto lexem_start = i;
            i++;
            Token::Category category;

            if (operator_or_delimiter != u"") {
                i = lexem_start + operator_or_delimiter.len();
                category = decltype(category)::OPERATOR_OR_DELIMITER;
                if (in(ch, u"([{"_S))
                    nesting_elements.append(make_tuple(ch, lexem_start));
                else if (in(ch, u")]}"_S)) {
                    if (nesting_elements.empty() || _get<0>(nesting_elements.last()) != ([&](const auto &a){return a == u')' ? u'('_C : a == u']' ? u'['_C : a == u'}' ? u'{'_C : throw KeyError(a);}(ch)))
                        throw Error(u"there is no corresponding opening parenthesis/bracket/brace for `"_S + ch + u"`"_S, lexem_start);
                    nesting_elements.pop();
                }
                else if (ch == u';')
                    category = decltype(category)::STATEMENT_SEPARATOR;
            }

            else if (in(ch, make_tuple(u"\""_S, u"'"_S)) || (in(ch, u"rRbB"_S) && in(source[range_el(i, i + 1)], make_tuple(u"\""_S, u"'"_S)))) {
                String ends;
                if (in(ch, u"rRbB"_S))
                    ends = in(source[range_el(i, i + 3)], make_tuple(u"\"\"\""_S, u"'''"_S)) ? source[range_el(i, i + 3)] : source[i];
                else {
                    i--;
                    ends = in(source[range_el(i, i + 3)], make_tuple(u"\"\"\""_S, u"'''"_S)) ? source[range_el(i, i + 3)] : ch;
                }
                i += ends.len();
                while (true) {
                    if (i == source.len())
                        throw Error(u"unclosed string literal"_S, lexem_start);
                    if (source[i] == u'\\') {
                        i++;
                        if (i == source.len())
                            continue;
                    }
                    else if (source[range_el(i, i + ends.len())] == ends) {
                        i += ends.len();
                        break;
                    }
                    i++;
                }
                category = decltype(category)::STRING_LITERAL;
            }

            else if (in(ch, range_ee(u'a'_C, u'z'_C)) || in(ch, range_ee(u'A'_C, u'Z'_C)) || ch == u'_') {
                while (i < source.len()) {
                    ch = source[i];
                    if (!(in(ch, range_ee(u'a'_C, u'z'_C)) || in(ch, range_ee(u'A'_C, u'Z'_C)) || ch == u'_' || in(ch, range_ee(u'0'_C, u'9'_C)) || ch == u'?'))
                        break;
                    i++;
                }
                if (in(source[range_el(lexem_start, i)], tokenizer::keywords)) {
                    if (in(source[range_el(lexem_start, i)], make_tuple(u"None"_S, u"False"_S, u"True"_S)))
                        category = decltype(category)::CONSTANT;
                    else
                        category = decltype(category)::KEYWORD;
                }
                else
                    category = decltype(category)::NAME;
            }

            else if ((in(ch, u"-+"_S) && in(source[range_el(i, i + 1)], range_ee(u'0'_C, u'9'_C))) || in(ch, range_ee(u'0'_C, u'9'_C))) {
                if (in(ch, u"-+"_S)) {
                    assert(false);
                    ch = source[i + 1];
                }
                else
                    i--;
                auto is_hex = ch == u'0' && in(source[range_el(i + 1, i + 2)], make_tuple(u"x"_S, u"X"_S));
                auto is_oct = ch == u'0' && in(source[range_el(i + 1, i + 2)], make_tuple(u"o"_S, u"O"_S));
                auto is_bin = ch == u'0' && in(source[range_el(i + 1, i + 2)], make_tuple(u"b"_S, u"B"_S));
                if (is_hex || is_oct || is_bin)
                    i += 2;
                auto start = i;
                i++;
                if (is_hex)
                    while (i < source.len() && (in(source[i], range_ee(u'0'_C, u'9'_C)) || in(source[i], range_ee(u'a'_C, u'z'_C)) || in(source[i], range_ee(u'A'_C, u'Z'_C)) || source[i] == u'_'))
                        i++;
                else if (is_oct)
                    while (i < source.len() && (in(source[i], range_ee(u'0'_C, u'7'_C)) || source[i] == u'_'))
                        i++;
                else if (is_bin)
                    while (i < source.len() && (in(source[i], u"01"_S) || source[i] == u'_'))
                        i++;
                else {
                    while (i < source.len() && (in(source[i], range_ee(u'0'_C, u'9'_C)) || in(source[i], u"_.eE"_S))) {
                        if (in(source[i], u"eE"_S)) {
                            if (in(source[range_el(i + 1, i + 2)], u"-+"_S))
                                i++;
                        }
                        i++;
                    }
                    if (in(u'_'_C, source[range_el(start, i)]) && !(in(u'.'_C, source[range_el(start, i)]))) {
                        auto number = source[range_el(start, i)].replace(u"_"_S, u""_S);
                        auto number_with_separators = u""_S;
                        auto j = number.len();
                        while (j > 3) {
                            number_with_separators = u"_"_S + number[range_el(j - 3, j)] + number_with_separators;
                            j -= 3;
                        }
                        number_with_separators = number[range_el(0, j)] + number_with_separators;
                        if (source[range_el(start, i)] != number_with_separators)
                            throw Error(u"digit separator in this number is located in the wrong place (should be: "_S + number_with_separators + u")"_S, start);
                    }
                }
                category = decltype(category)::NUMERIC_LITERAL;
            }

            else if (ch == u'\\') {
                if (!in(source[i], u"\r\n"_S))
                    throw Error(u"only new line character allowed after backslash"_S, i);
                if (source[i] == u'\r')
                    i++;
                if (source[i] == u'\n')
                    i++;
                continue;
            }
            else
                throw Error(u"unexpected character "_S + ch, lexem_start);
            tokens.append(Token(lexem_start, i, category));
        }
    }
    if (!nesting_elements.empty())
        throw Error(u"there is no corresponding closing parenthesis/bracket/brace for `"_S + _get<0>(nesting_elements.last()) + u"`"_S, _get<1>(nesting_elements.last()));
    if (expected_an_indented_block)
        throw Error(u"expected an indented block"_S, i);

    while (indentation_levels.len()) {
        tokens.append(Token(i, i, Token::Category::DEDENT));
        indentation_levels.pop();
    }
    return tokens;
}
