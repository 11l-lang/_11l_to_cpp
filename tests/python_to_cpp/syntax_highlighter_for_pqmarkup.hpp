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
    Array<Tuple<int, int>> comments;

    if (lang == u"Python") {
        for (auto token : python_to_11l::tokenizer::tokenize(source, nullptr, &comments))
            print(token.to_str(source));
        print(comments);
    }
    else
        assert(lang == u"11l");
    return source;
}
