auto operators = create_array({u"+"_S, u"-"_S, u"*"_S, u"**"_S, u"/"_S, u"//"_S, u"%"_S, u"@"_S, u"<<"_S, u">>"_S, u"&"_S, u"|"_S, u"^"_S, u"~"_S, u"<"_S, u">"_S, u"<="_S, u">="_S, u"=="_S, u"!="_S});
auto delimiters = create_array({u"("_S, u")"_S, u"["_S, u"]"_S, u"{"_S, u"}"_S, u","_S, u":"_S, u"."_S, u";"_S, u"@"_S, u"="_S, u"->"_S, u"+="_S, u"-="_S, u"*="_S, u"/="_S, u"//="_S, u"%="_S, u"@="_S, u"&="_S, u"|="_S, u"^="_S, u">>="_S, u"<<="_S, u"**="_S});
auto operators_and_delimiters = sorted(operators + delimiters, [](const auto &x){return x.len();}, true);

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
    if (comments != nullptr)
        comments->append(make_tuple(10, 20));
    tokens.append(Token(0, source.len(), Token::Category::STATEMENT_SEPARATOR));
    return tokens;
}
