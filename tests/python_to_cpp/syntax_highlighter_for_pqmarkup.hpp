namespace python_to_11l { namespace tokenizer {
#include "python_to_11l/tokenizer.hpp"
}}
namespace _11l_to_cpp { namespace tokenizer {
#include "_11l_to_cpp/tokenizer.hpp"
}}

auto css = uR"(<style>
span.keyword {color: #0000FF; font-weight: bold;}
span.identifier {color: #00009F;}
span.string-literal {color: #800000;}
span.numeric-literal {color: #008000;}
span.constant {color: #008000;}
span.comment {color: #808080;}
</style>)"_S;
auto cat_to_class_python = create_dict(dict_of(python_to_11l::tokenizer::Token::Category::NAME, u"identifier"_S)(python_to_11l::tokenizer::Token::Category::KEYWORD, u"keyword"_S)(python_to_11l::tokenizer::Token::Category::CONSTANT, u"constant"_S)(python_to_11l::tokenizer::Token::Category::OPERATOR_OR_DELIMITER, u""_S)(python_to_11l::tokenizer::Token::Category::NUMERIC_LITERAL, u"numeric-literal"_S)(python_to_11l::tokenizer::Token::Category::STRING_LITERAL, u"string-literal"_S)(python_to_11l::tokenizer::Token::Category::INDENT, u""_S)(python_to_11l::tokenizer::Token::Category::DEDENT, u""_S)(python_to_11l::tokenizer::Token::Category::STATEMENT_SEPARATOR, u""_S));
auto cat_to_class_11l = create_dict(dict_of(_11l_to_cpp::tokenizer::Token::Category::NAME, u"identifier"_S)(_11l_to_cpp::tokenizer::Token::Category::KEYWORD, u"keyword"_S)(_11l_to_cpp::tokenizer::Token::Category::CONSTANT, u"constant"_S)(_11l_to_cpp::tokenizer::Token::Category::DELIMITER, u""_S)(_11l_to_cpp::tokenizer::Token::Category::OPERATOR, u""_S)(_11l_to_cpp::tokenizer::Token::Category::NUMERIC_LITERAL, u"numeric-literal"_S)(_11l_to_cpp::tokenizer::Token::Category::STRING_LITERAL, u"string-literal"_S)(_11l_to_cpp::tokenizer::Token::Category::STRING_CONCATENATOR, u""_S)(_11l_to_cpp::tokenizer::Token::Category::SCOPE_BEGIN, u""_S)(_11l_to_cpp::tokenizer::Token::Category::SCOPE_END, u""_S)(_11l_to_cpp::tokenizer::Token::Category::STATEMENT_SEPARATOR, u""_S));

template <typename T1> auto is_lang_supported(const T1 &lang)
{
    return in(lang, make_tuple(u"11l"_S, u"Python"_S));
}

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
    else {
        assert(lang == u"11l");
        try
        {
            for (auto token : _11l_to_cpp::tokenizer::tokenize(source, nullptr, nullptr, &comments) + create_array({_11l_to_cpp::tokenizer::Token(source.len(), source.len(), _11l_to_cpp::tokenizer::Token::Category::STATEMENT_SEPARATOR)})) {
                while (comments.len() && _get<0>(_get<0>(comments)) < token.start) {
                    res += html_escape(source[range_el(writepos, _get<0>(_get<0>(comments)))]);
                    writepos = _get<1>(_get<0>(comments));
                    res += u"<span class=\"comment\">"_S + html_escape(source[range_el(_get<0>(_get<0>(comments)), _get<1>(_get<0>(comments)))]) + u"</span>"_S;
                    comments.pop(0);
                }
                res += html_escape(source[range_el(writepos, token.start)]);
                writepos = token.end;
                auto tokstr = html_escape(token.value(source));
                String css_class;
                if ((token.category == decltype(token.category)::NAME && in(tokstr, make_tuple(u"V"_S, u"П"_S, u"var"_S, u"перем"_S))) || (token.category == decltype(token.category)::OPERATOR && in(tokstr, make_tuple(u"C"_S, u"С"_S, u"in"_S, u"!C"_S, u"!С"_S, u"!in"_S))) || in(_get<0>(tokstr.split(u"."_S)), _11l_to_cpp::tokenizer::keywords))
                    css_class = u"keyword"_S;
                else
                    css_class = syntax_highlighter_for_pqmarkup::cat_to_class_11l[token.category];
                if (css_class != u"")
                    res += u"<span class=\""_S + css_class + u"\">"_S + tokstr + u"</span>"_S;
                else
                    res += tokstr;
            }
        }

        catch (const _11l_to_cpp::tokenizer::Error& e)
        {
            throw Error(e.message, e.pos);
        }
    }
    return res;
}
