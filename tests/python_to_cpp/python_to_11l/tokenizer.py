from typing import List, Tuple, Optional
Char = str
from enum import IntEnum

keywords = [ # https://docs.python.org/3/reference/lexical_analysis.html#keywords
            'False',      'await',      'else',       'import',     'pass',
            'None',       'break',      'except',     'in',         'raise',
            'True',       'class',      'finally',    'is',         'return',
            'and',        'continue',   'for',        'lambda',     'try',
            'as',         'def',        'from',       'nonlocal',   'while',
            'assert',     'del',        'global',     'not',        'with',
            'async',      'elif',       'if',         'or',         'yield',]

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

class Error(Exception):
    message : str
    pos : int
    end : int

    def __init__(self, message, pos):
        self.message = message
        self.pos = pos
        self.end = pos

class Token:
    class Category(IntEnum): # why ‘Category’: >[https://docs.python.org/3/reference/lexical_analysis.html#other-tokens]:‘the following categories of tokens exist’
        NAME = 0 # or IDENTIFIER
        KEYWORD = 1
        CONSTANT = 2
        OPERATOR_OR_DELIMITER = 3
        NUMERIC_LITERAL = 4
        STRING_LITERAL = 5
        FSTRING = 6
        FSTRING_END = 7 # this is needed for syntax highlighting
        INDENT = 8 # [https://docs.python.org/3/reference/lexical_analysis.html#indentation][-1]
        DEDENT = 9
        STATEMENT_SEPARATOR = 10

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

def tokenize(source, newline_chars : List[int] = None, comments : List[Tuple[int, int]] = None) -> List[Token]:
    tokens : List[Token] = []
    indentation_levels : List[int] = []
    nesting_elements : List[Tuple[Char, int]] = [] # parentheses, square brackets or curly braces

    begin_of_line = True
    expected_an_indented_block = False
    i = 0

    while i < len(source):
        if begin_of_line: # at the beginning of each line, the line's indentation level is compared to the last of the indentation_levels [:1]
            begin_of_line = False
            linestart = i
            indentation_level = 0
            while i < len(source):
                if source[i] == ' ':
                    indentation_level += 1
                elif source[i] == "\t":
                    indentation_level += 8 # consider tab as just 8 spaces (I know that Python 3 use different rules, but I disagree with Python 3 approach ([-1]:‘Tabs are replaced (from left to right) by one to eight spaces’), so I decided to use this simpler solution)
                else:
                    break
                i += 1
            if i == len(source): # end of source
                break

            if source[i] in "\r\n#": # lines with only whitespace and/or comments do not affect the indentation
                continue

            prev_indentation_level = indentation_levels[-1] if len(indentation_levels) else 0

            if expected_an_indented_block:
                if not indentation_level > prev_indentation_level:
                    raise Error('expected an indented block', i)

            if indentation_level == prev_indentation_level: # [1:] [-1]:‘If it is equal, nothing happens.’ [:2]
                if len(tokens):
                    tokens.append(Token(linestart-1, linestart, Token.Category.STATEMENT_SEPARATOR))
            elif indentation_level > prev_indentation_level: # [2:] [-1]:‘If it is larger, it is pushed on the stack, and one INDENT token is generated.’ [:3]
                if not expected_an_indented_block:
                    raise Error('unexpected indent', i)
                expected_an_indented_block = False
                indentation_levels.append(indentation_level)
                tokens.append(Token(linestart, i, Token.Category.INDENT))
            else: # [3:] [-1]:‘If it is smaller, it ~‘must’ be one of the numbers occurring on the stack; all numbers on the stack that are larger are popped off, and for each number popped off a DEDENT token is generated.’ [:4]
                while True:
                    indentation_levels.pop()
                    tokens.append(Token(i, i, Token.Category.DEDENT))
                    level = indentation_levels[-1] if len(indentation_levels) else 0
                    if level == indentation_level:
                        break
                    if level < indentation_level:
                        raise Error('unindent does not match any outer indentation level', i)

        ch = source[i]

        if ch in " \t":
            i += 1 # just skip whitespace characters
        elif ch in "\r\n":
            if newline_chars is not None:
                newline_chars.append(i)
            i += 1
            if ch == "\r" and source[i:i+1] == "\n":
                i += 1
            if len(nesting_elements) == 0: # [https://docs.python.org/3/reference/lexical_analysis.html#implicit-line-joining ‘Implicit line joining’]:‘Expressions in parentheses, square brackets or curly braces can be split over more than one physical line without using backslashes.’
                begin_of_line = True
        elif ch == '#':
            comment_start = i
            i += 1
            while i < len(source) and source[i] not in "\r\n":
                i += 1
            if comments is not None:
                comments.append((comment_start, i))
        else:
            expected_an_indented_block = ch == ':'

            operator_or_delimiter = ''
            for op in operators_and_delimiters:
                if source[i:i+len(op)] == op:
                    if op == '.' and source[i+1:i+2].isdigit():
                        break
                    operator_or_delimiter = op
                    break

            lexem_start = i
            i += 1
            category : Token.Category

            if operator_or_delimiter != '':
                i = lexem_start + len(operator_or_delimiter)
                category = Token.Category.OPERATOR_OR_DELIMITER
                if ch in '([{':
                    nesting_elements.append((ch, lexem_start))
                elif ch in ')]}': # ([{
                    if len(nesting_elements) == 0 or nesting_elements[-1][0] != {')':'(', ']':'[', '}':'{'}[ch]: # }])
                        raise Error('there is no corresponding opening parenthesis/bracket/brace for `' + ch + '`', lexem_start)
                    nesting_elements.pop()
                elif ch == ';':
                    category = Token.Category.STATEMENT_SEPARATOR

            elif ch in ('"', "'") or (ch in 'rRbBfF' and source[i:i+1] in ('"', "'")):
                ends : str
                if ch in 'rRbBfF':
                    ends = source[i:i+3] if source[i:i+3] in ('"""', "'''") else source[i]
                else:
                    i -= 1
                    ends = source[i:i+3] if source[i:i+3] in ('"""', "'''") else ch
                i += len(ends)
                while True:
                    if i == len(source):
                        raise Error('unclosed string literal', lexem_start)
                    if source[i] == '\\':
                        i += 1
                        if i == len(source):
                            continue
                    elif source[i:i+len(ends)] == ends:
                        i += len(ends)
                        break
                    i += 1

                if ch in 'fF':
                    tokens.append(Token(lexem_start, lexem_start + 1 + len(ends), Token.Category.FSTRING))
                    #tokens.append(Token(i, i, Token.Category.FSTRING)) # second FSTRING token is needed for parsing f-strings starting with expression [otherwise there is an error: no symbol corresponding to token `` (belonging to Category.INDENT) found while parsing expression]
                    j = lexem_start + 1 + len(ends)
                    substr_start = j
                    while j < i - len(ends):
                        if source[j] == '{':
                            if source[j+1] == '{':
                                j += 2
                                continue
                            if j > substr_start:
                                tokens.append(Token(substr_start, j, Token.Category.STRING_LITERAL))
                            tokens.append(Token(j, j, Token.Category.INDENT))
                            j += 1
                            s = j
                            colon_pos : Optional[int] = None
                            nesting_level = 0
                            while True:
                                if source[j] == '{':
                                    nesting_level += 1
                                elif source[j] == '}':
                                    if nesting_level == 0:
                                        break
                                    nesting_level -= 1
                                elif source[j] == ':' and nesting_level == 0:
                                    colon_pos = j
                                j += 1
                            for new_token in tokenize(source[s:colon_pos if colon_pos is not None else j]):
                                tokens.append(Token(new_token.start + s, new_token.end + s, new_token.category))
                            if colon_pos is not None:
                                #tokens.append(Token(colon_pos, colon_pos, Token.Category.STATEMENT_SEPARATOR))
                                #tokens.append(Token(colon_pos + 1, j, Token.Category.STRING_LITERAL))
                                tokens.append(Token(colon_pos + 1, j, Token.Category.STATEMENT_SEPARATOR))
                            tokens.append(Token(j, j, Token.Category.DEDENT))
                            substr_start = j + 1
                        elif source[j] == '}':
                            if source[j+1] != '}': # {
                                raise Error("f-string: single '}' is not allowed", j)
                            j += 2
                            continue
                        j += 1
                    if j > substr_start:
                        tokens.append(Token(substr_start, j, Token.Category.STRING_LITERAL))
                    tokens.append(Token(j, i, Token.Category.FSTRING_END))
                    continue

                category = Token.Category.STRING_LITERAL

            elif ch.isalpha() or ch == '_': # this is NAME/IDENTIFIER or KEYWORD
                while i < len(source):
                    ch = source[i]
                    if not (ch.isalpha() or ch == '_' or '0' <= ch <= '9' or ch == '?'):
                        break
                    i += 1
                if source[lexem_start:i] in keywords:
                    if source[lexem_start:i] in ('None', 'False', 'True'):
                        category = Token.Category.CONSTANT
                    else:
                        category = Token.Category.KEYWORD
                else:
                    category = Token.Category.NAME

            elif (ch in '-+' and '0' <= source[i:i+1] <= '9') or '0' <= ch <= '9' or (ch == '.' and '0' <= source[i:i+1] <= '9'): # this is NUMERIC_LITERAL
                if ch in '-+':
                    assert(False) # considering sign as a part of numeric literal is a bad idea — expressions like `j-3` are cease to parse correctly
                    #sign = ch
                    ch = source[i+1]
                else:
                    i -= 1
                is_hex = ch == '0' and source[i+1:i+2] in ('x', 'X')
                is_oct = ch == '0' and source[i+1:i+2] in ('o', 'O')
                is_bin = ch == '0' and source[i+1:i+2] in ('b', 'B')
                if is_hex or is_oct or is_bin:
                    i += 2
                    # if not '0' <= source[i:i+1] <= '9':
                    #     raise Error('expected digit', i)
                start = i
                i += 1
                if is_hex:
                    while i < len(source) and ('0' <= source[i] <= '9' or 'a' <= source[i] <= 'f' or 'A' <= source[i] <= 'F' or source[i] == '_'):
                        i += 1
                elif is_oct:
                    while i < len(source) and ('0' <= source[i] <= '7' or source[i] == '_'):
                        i += 1
                elif is_bin:
                    while i < len(source) and source[i] in '01_':
                        i += 1
                else:
                    while i < len(source) and ('0' <= source[i] <= '9' or source[i] in '_.eE'):
                        if source[i] in 'eE':
                            if source[i+1:i+2] in '-+':
                                i += 1
                        i += 1
                    if source[i:i+1] in ('j', 'J'):
                        i += 1
                    if '_' in source[start:i] and not '.' in source[start:i]: # float numbers do not checked for a while
                        number = source[start:i].replace('_', '')
                        number_with_separators = ''
                        j = len(number)
                        while j > 3:
                            number_with_separators = '_' + number[j-3:j] + number_with_separators
                            j -= 3
                        number_with_separators = number[0:j] + number_with_separators
                        if source[start:i] != number_with_separators:
                            raise Error('digit separator in this number is located in the wrong place (should be: '+ number_with_separators +')', start)
                category = Token.Category.NUMERIC_LITERAL

            elif ch == '\\':
                if source[i] not in "\r\n":
                    raise Error('only new line character allowed after backslash', i)
                if source[i] == "\r":
                    i += 1
                if source[i] == "\n":
                    i += 1
                continue

            else:
                raise Error('unexpected character ' + ch, lexem_start)

            tokens.append(Token(lexem_start, i, category))

    if len(nesting_elements):
        raise Error('there is no corresponding closing parenthesis/bracket/brace for `' + nesting_elements[-1][0] + '`', nesting_elements[-1][1])

    if expected_an_indented_block:
        raise Error('expected an indented block', i)

    while len(indentation_levels): # [4:] [-1]:‘At the end of the file, a DEDENT token is generated for each number remaining on the stack that is larger than zero.’
        tokens.append(Token(i, i, Token.Category.DEDENT))
        indentation_levels.pop()

    return tokens
