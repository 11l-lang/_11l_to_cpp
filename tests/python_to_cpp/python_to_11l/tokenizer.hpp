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
};
