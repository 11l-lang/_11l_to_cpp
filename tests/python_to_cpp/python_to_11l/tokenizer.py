from enum import IntEnum

class Token:
    class Category(IntEnum):
        NAME = 0 # or IDENTIFIER
        KEYWORD = 1
        CONSTANT = 2
        OPERATOR_OR_DELIMITER = 3
        NUMERIC_LITERAL = 4
        STRING_LITERAL = 5
        INDENT = 6 # [https://docs.python.org/3/reference/lexical_analysis.html#indentation][-1]
        DEDENT = 7
        STATEMENT_SEPARATOR = 8
