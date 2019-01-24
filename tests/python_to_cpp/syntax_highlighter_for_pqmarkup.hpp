namespace python_to_11l { namespace tokenizer {
#include "python_to_11l/tokenizer.hpp"
}}

auto cat_to_class_python = create_dict(dict_of(python_to_11l::tokenizer::Token::Category::NAME, u"identifier"_S)(python_to_11l::tokenizer::Token::Category::KEYWORD, u"keyword"_S)(python_to_11l::tokenizer::Token::Category::CONSTANT, u"constant"_S)(python_to_11l::tokenizer::Token::Category::OPERATOR_OR_DELIMITER, u""_S)(python_to_11l::tokenizer::Token::Category::NUMERIC_LITERAL, u"numeric-literal"_S)(python_to_11l::tokenizer::Token::Category::STRING_LITERAL, u"string-literal"_S)(python_to_11l::tokenizer::Token::Category::INDENT, u""_S)(python_to_11l::tokenizer::Token::Category::DEDENT, u""_S)(python_to_11l::tokenizer::Token::Category::STATEMENT_SEPARATOR, u""_S));

class Error
{
public:
    String message;
    int pos;
    template <typename T1, typename T2> Error(const T1 &message, const T2 &pos)
    {
        this->message = message;
        this->pos = pos;
    }
};

template <typename T1, typename T2> auto highlight(const T1 &lang, const T2 &source)
{
    auto writepos = 0;
    Array<Tuple<int, int>> comments;
    auto res = u""_S;

    auto html_escape = [](const auto &s)
    {
        return s.replace(u"&"_S, u"&amp;"_S).replace(u"<"_S, u"&lt;"_S);
    };

    if (lang == u"Python") {
        try
        {
            for (auto token : python_to_11l::tokenizer::tokenize(source, nullptr, &comments) + create_array({python_to_11l::tokenizer::Token(source.len(), source.len(), python_to_11l::tokenizer::Token::Category::STATEMENT_SEPARATOR)})) {
                while (comments.len() && _get<0>(_get<0>(comments)) < token.start) {
                    res += html_escape(source[range_el(writepos, _get<0>(_get<0>(comments)))]);
                    writepos = _get<1>(_get<0>(comments));
                    res += u"<span class=\"comment\">"_S + html_escape(source[range_el(_get<0>(_get<0>(comments)), _get<1>(_get<0>(comments)))]) + u"</span>"_S;
                    comments.pop(0);
                }
                res += html_escape(source[range_el(writepos, token.start)]);
                writepos = token.end;
                auto css_class = syntax_highlighter_for_pqmarkup::cat_to_class_python[token.category];
                if (css_class != u"")
                    res += u"<span class=\""_S + css_class + u"\">"_S + html_escape(token.value(source)) + u"</span>"_S;
                else
                    res += html_escape(token.value(source));
            }
        }

        catch (const python_to_11l::tokenizer::Error& e)
        {
            throw Error(e.message, e.pos);
        }
    }
    else
        assert(lang == u"11l");
    return res;
}
