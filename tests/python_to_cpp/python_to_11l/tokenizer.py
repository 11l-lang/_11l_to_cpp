from typing import List, Tuple
from enum import IntEnum

operators = [ # https://docs.python.org/3/reference/lexical_analysis.html#operators
             '+',       '-',       '*',       '**',      '/',       '//',      '%',      '@',
             '<<',      '>>',      '&',       '|',       '^',       '~',
             '<',       '>',       '<=',      '>=',      '==',      '!=',]
#operators.sort(key = lambda x: len(x), reverse = True)

delimiters = [ # https://docs.python.org/3/reference/lexical_analysis.html#delimiters        
              '(',       ')',       '[',       ']',       '{',       '}',
              ',',       ':',       '.',       ';',       '@',       '=',       '->',
              '+=',      '-=',      '*=',      '/=',      '//=',     '%=',      '@=',
              '&=',      '|=',      '^=',      '>>=',     '<<=',     '**=',]
#delimiters.sort(key = lambda x: len(x), reverse = True)
operators_and_delimiters = sorted(operators + delimiters, key = lambda x: len(x), reverse = True)

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

    start : int
    end : int
    category : Category

    def __init__(self, start, end, category):
        self.start = start
        self.end = end
        self.category = category

    def __repr__(self):
        return str(self.start)

    def value(self, source):
        return source[self.start:self.end]

    def to_str(self, source):
        return 'Token('+str(self.category)+', "'+self.value(source)+'")'

def tokenize(source, newline_chars : List[int] = None, comments : List[Tuple[int, int]] = None):
    tokens : List[Token] = []

    if comments != None:
        comments.append((10, 20))
    tokens.append(Token(0, len(source), Token.Category.STATEMENT_SEPARATOR))

    return tokens
