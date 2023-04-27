"""
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
"""
from enum import IntEnum
from typing import List, Set, Tuple, Optional
Char = str

keywords = ['V',   'C',  'I',    'E',     'F',  'L',    'N',    'R',       'S',       'T',    'X',
            'П',   'С',  'Е',    'И',     'Ф',  'Ц',    'Н',    'Р',       'В',       'Т',    'Х',
            'var', 'in', 'if',   'else',  'fn', 'loop', 'null', 'return',  'switch',  'type', 'exception',
            'пер', 'св', 'если', 'иначе', 'фн', 'цикл', 'нуль', 'вернуть', 'выбрать', 'тип',  'исключение']
#keywords.remove('C'); keywords.remove('С'); keywords.remove('in'); keywords.remove('св') # it is more convenient to consider C/in as an operator, not a keyword (however, this line is not necessary)
empty_list_of_str : List[str] = []
binary_operators : List[Set[str]] = [] # `initializer_list` does not support move-only types (like `Set`) ([https://stackoverflow.com/questions/8193102/initializer-list-and-move-semantics <- google:‘initializer_list rvalue’])
binary_operators.append(set(empty_list_of_str))
binary_operators.append({str('+'), '-', '*', '/', '%', '^', '&', '|', '<', '>', '=', '?'})
binary_operators.append({'<<', '>>', '<=', '>=', '==', '!=', '+=', '-=', '*=', '/=', '%=', '&=', '|=', '^=', '.=', '->', '..', '.<', '.+', '<.', 'I/', 'Ц/', '-%', 'C ', 'С '})
binary_operators.append({'<<=', '>>=', '‘’=', '[+]', '[&]', '[|]', '(+)', '<.<', '-I/', '-Ц/', 'I/=', 'Ц/=', 'in ', 'св ', '!C ', '!С '})
binary_operators.append({'[+]=', '[&]=', '[|]=', '(+)=', '!in ', '!св '})
unary_operators : List[Set[str]] = []
unary_operators.append(set(empty_list_of_str))
unary_operators.append({str('!')})
unary_operators.append({'++', '--'})
unary_operators.append({'(-)'})
unary_operators.append(set(empty_list_of_str))
all_operators = set() # str
operators : List[Set[str]] = []
for i in range(5):
    operators.append(binary_operators[i] | unary_operators[i])
    all_operators |= binary_operators[i] | unary_operators[i]
first_char_of_operators = set() # Char
for op in all_operators:
    first_char_of_operators.add(op[0])
binary_operators[1].remove('^') # for `^L.break` support
binary_operators[2].remove('..') # for `L(n) 1..`


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
        DELIMITER = 3 # SEPARATOR = 3
        OPERATOR = 4
        NUMERIC_LITERAL = 5
        STRING_LITERAL = 6
        STRING_CONCATENATOR = 7 # special token inserted between adjacent string literal and some identifier
        FSTRING = 8
        FSTRING_END = 9 # this is needed for syntax highlighting
        SCOPE_BEGIN = 10 # similar to ‘INDENT token in Python’[https://docs.python.org/3/reference/lexical_analysis.html][-1]
        SCOPE_END   = 11 # similar to ‘DEDENT token in Python’[-1]
        STATEMENT_SEPARATOR = 12

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

def tokenize(source : str, implied_scopes : List[Tuple[Char, int]] = None, line_continuations : List[int] = None, comments : List[Tuple[int, int]] = None) -> List[Token]:
    tokens : List[Token] = []
    indentation_levels : List[Tuple[int, bool]] = []
    nesting_elements : List[Tuple[Char, int]] = [] # логически этот стек можно объединить с indentation_levels, но так немного удобнее (конкретно: для проверок `nesting_elements[-1][0] != ...`)
    i = 0
    begin_of_line = True
    indentation_tabs : bool
    prev_linestart : int

    def skip_multiline_comment():
        nonlocal i, source, comments
        comment_start = i
        lbr = source[i+1]
        rbr = {"‘": "’", "(": ")", "{": "}", "[": "]"}[lbr]
        i += 2
        nesting_level = 1
        while True:
            ch = source[i]
            i += 1
            if ch == lbr:
                nesting_level += 1
            elif ch == rbr:
                nesting_level -= 1
                if nesting_level == 0:
                    break
            if i == len(source):
                raise Error('there is no corresponding opening parenthesis/bracket/brace/qoute for `' + lbr + '`', comment_start+1)
        if comments is not None:
            comments.append((comment_start, i))

    while i < len(source):
        if begin_of_line: # at the beginning of each line, the line's indentation level is compared to the last indentation_levels [:1]
            begin_of_line = False
            linestart = i
            tabs = False
            spaces = False
            while i < len(source):
                if source[i] == ' ':
                    spaces = True
                elif source[i] == "\t":
                    tabs = True
                else:
                    break
                i += 1
            if i == len(source): # end of source
                break

            ii = i
            if source[i:i+2] in (R'\‘', R'\(', R'\{', R'\['): # ]})’
                skip_multiline_comment()
                while i < len(source) and source[i] in " \t": # skip whitespace characters
                    i += 1
                if i == len(source): # end of source
                    break

            if source[i] in "\r\n" or source[i:i+2] in ('//', R'\\'): # lines with only whitespace and/or comments do not affect the indentation
                continue

            if source[i] in "{}": # Indentation level of lines starting with { or } is ignored
                continue

            if len(tokens) \
               and tokens[-1].category == Token.Category.STRING_CONCATENATOR \
               and source[i] in '"\'‘': # ’ and not source[i+1:i+2] in ({'"':'"', '‘':'’'}[source[i]],):
                if line_continuations is not None:
                    line_continuations.append(tokens[-1].end)
                if source[i:i+2] in ('""', '‘’'):
                    i += 2
                continue

            if len(tokens) \
               and tokens[-1].category == Token.Category.STRING_LITERAL \
               and source[i:i+2] in ('""', '‘’'):
                if line_continuations is not None:
                    line_continuations.append(tokens[-1].end)
                tokens.append(Token(i, i, Token.Category.STRING_CONCATENATOR))
                i += 2
                continue

            if (len(tokens)
                and tokens[-1].category == Token.Category.OPERATOR
                and tokens[-1].value(source) in binary_operators[tokens[-1].end - tokens[-1].start] # ‘Every line of code which ends with any binary operator should be joined with the following line of code.’:[https://github.com/JuliaLang/julia/issues/2097#issuecomment-339924750][-339924750]<
                and source[tokens[-1].end-4:tokens[-1].end] != '-> &' # for `F symbol(id, bp = 0) -> &`
                and (tokens[-1].value(source) != '?' or source[tokens[-1].start-1] == ' ')): # for `F score(board = board) -> (Char, [Int])?`
                if line_continuations is not None:
                    line_continuations.append(tokens[-1].end)
                continue

            # if not (len(indentation_levels) and indentation_levels[-1][0] == -1): # сразу после символа `{` это [:правило] не действует ...а хотя не могу подобрать пример, который бы показывал необходимость такой проверки, а потому оставлю этот if закомментированным # }
            if ((source[i    ] in binary_operators[1]
              or source[i:i+2] in binary_operators[2]
              or source[i:i+3] in binary_operators[3]
              or source[i:i+4] in binary_operators[4]) # [правило:] ‘Every line of code which begins with any binary operator should be joined with the previous line of code.’:[-339924750]<
              and not (source[i    ] in unary_operators[1]  # Rude fix for:
                    or source[i:i+2] in unary_operators[2]  # a=b
                    or source[i:i+3] in unary_operators[3]) # ++i // Plus symbol at the beginning here should not be treated as binary + operator, so there is no implied line joining
              and (source[i] not in ('&', '-') or source[i+1:i+2] == ' ') # Символы `&` и `-` обрабатываются по-особенному — склеивание строк происходит только если после одного из этих символов стоит пробел
              and source[i] not in ('<', '>')): # for `<`/`<=`/`>`/`>=` cases in switch
                if len(tokens) == 0:
                    raise Error('source can not starts with a binary operator', i)
                if line_continuations is not None:
                    line_continuations.append(tokens[-1].end)
                continue

            if source[i:i+2] == R'\.': # // Support for constructions like: ||| You need just to add `\` at the each line starting with dot:
                if len(tokens):        # \\ result = abc.method1()          ||| result = abc.method1()
                    i += 1             # \\             .method2()          |||            \.method2()
               #else: # with `if len(tokens): i += 1` there is no need for this else branch
               #    raise Error('unexpected character `\`')
                    if line_continuations is not None:
                        line_continuations.append(tokens[-1].end)
                continue

            if tabs and spaces:
                next_line_pos = source.find("\n", i)
                raise Error('mixing tabs and spaces in indentation: `' + source[linestart:i].replace(' ', 'S').replace("\t", 'TAB') + source[i:next_line_pos if next_line_pos != -1 else len(source)] + '`', i)

            indentation_level = ii - linestart
            if len(indentation_levels) and indentation_levels[-1][0] == -1: # сразу после символа `{` идёт новый произвольный отступ (понижение уровня отступа может быть полезно, если вдруг отступ оказался слишком большой), который действует вплоть до парного символа `}`
                assert(indentation_levels[-1][1])
                indentation_levels[-1] = (indentation_level, True) #indentation_levels[-1][0] = indentation_level # || maybe this is unnecessary (actually it is necessary, see test "fn f()\n{\na = 1") // }
                indentation_tabs = tabs
            else:
                prev_indentation_level = indentation_levels[-1][0] if len(indentation_levels) else 0

                if indentation_level > 0 and prev_indentation_level > 0 and indentation_tabs != tabs:
                    e = i + 1
                    while e < len(source) and source[e] not in "\r\n":
                        e += 1
                    raise Error("inconsistent indentations:\n```\n" + prev_indentation_level*('TAB' if indentation_tabs else 'S') + source[prev_linestart:linestart]
                        + (ii-linestart)*('TAB' if tabs else 'S') + source[ii:e] + "\n```", ii)
                prev_linestart = ii

                if indentation_level == prev_indentation_level: # [1:] [-1]:‘If it is equal, nothing happens.’ :)(: [:2]
                    if len(tokens) and tokens[-1].category != Token.Category.SCOPE_END:
                        tokens.append(Token(linestart-1, linestart, Token.Category.STATEMENT_SEPARATOR))
                elif indentation_level > prev_indentation_level: # [2:] [-1]:‘If it is larger, it is pushed on the stack, and one INDENT token is generated.’ [:3]
                    if prev_indentation_level == 0: # len(indentation_levels) == 0 or indentation_levels[-1][0] == 0:
                        indentation_tabs = tabs # первоначальная/новая установка символа для отступа (либо табуляция, либо пробелы) производится только от нулевого уровня отступа
                    indentation_levels.append((indentation_level, False))
                    tokens.append(Token(linestart, ii, Token.Category.SCOPE_BEGIN))
                    if implied_scopes is not None:
                        implied_scopes.append((Char('{'), tokens[-2].end + (1 if source[tokens[-2].end] in " \n" else 0)))
                else: # [3:] [-1]:‘If it is smaller, it ~‘must’ be one of the numbers occurring on the stack; all numbers on the stack that are larger are popped off, and for each number popped off a DEDENT token is generated.’ [:4]
                    while True:
                        if indentation_levels[-1][1]:
                            raise Error('too much unindent, what is this unindent intended for?', ii)
                        indentation_levels.pop()
                        tokens.append(Token(ii, ii, Token.Category.SCOPE_END))
                        if implied_scopes is not None:
                            implied_scopes.append((Char('}'), ii))
                        level = indentation_levels[-1][0] if len(indentation_levels) else 0 #level, explicit_scope_via_curly_braces = indentation_levels[-1] if len(indentation_levels) else [0, False]
                        if level == indentation_level:
                            break
                        if level < indentation_level:
                            raise Error('unindent does not match any outer indentation level', ii)

        ch = source[i]

        if ch in " \t":
            i += 1 # just skip whitespace characters
        elif ch in "\r\n":
            #if newline_chars is not None: # rejected this code as it does not count newline characters inside comments and string literals
            #    newline_chars.append(i)
            i += 1
            if ch == "\r" and source[i:i+1] == "\n":
                i += 1
            if len(nesting_elements) == 0 or nesting_elements[-1][0] not in '([': # если мы внутри скобок, то начинать новую строку не нужно # ])
                begin_of_line = True
        elif (ch == '/'  and source[i+1:i+2] == '/' ) \
          or (ch == '\\' and source[i+1:i+2] == '\\'): # single-line comment
            comment_start = i
            i += 2
            while i < len(source) and source[i] not in "\r\n":
                i += 1
            if comments is not None:
                comments.append((comment_start, i))
        elif ch == '\\' and source[i+1:i+2] in "‘({[": # multi-line comment # ]})’
            skip_multiline_comment()
        else:
            def is_hexadecimal_digit(ch):
                return '0' <= ch <= '9' or 'A' <= ch <= 'F' or 'a' <= ch <= 'f' or ch in 'абсдефАБСДЕФ'

            operator_s = ''
            # if ch in 'CС' and not (source[i+1:i+2].isalpha() or source[i+1:i+2].isdigit()): # without this check [and if 'C' is in binary_operators] when identifier starts with `C` (for example `Circle`), then this first letter of identifier is mistakenly considered as an operator
            #     operator_s = ch
            # else:
            if ch in first_char_of_operators:
                for n in range(4, 0, -1):
                    if source[i:i+n] in operators[n]:
                        operator_s = source[i:i+n]
                        if operator_s == '|' and source[i+1:i+2] in ('‘', "'"): # ’ # this is an indented multi-line string literal
                            operator_s = ''
                        break

            lexem_start = i
            i += 1
            category : Token.Category

            if operator_s != '':
                i = lexem_start + len(operator_s)
                if source[i-1] == ' ': # for correct handling of operator 'C '/'in ' in external tools (e.g. keyletters_to_keywords.py)
                    i -= 1
                category = Token.Category.OPERATOR

            elif ch.isalpha() or ch in ('_', '@'): # this is NAME/IDENTIFIER or KEYWORD
                if ch == '@':
                    while i < len(source) and source[i] == '@':
                        i += 1
                    if i < len(source) and source[i] in ('=', ':'):
                        i += 1
                while i < len(source):
                    ch = source[i]
                    if not (ch.isalpha() or ch in '_?:' or '0' <= ch <= '9'):
                        break
                    i += 1
                # Tokenize `fs:path:dirname` to ['fs:path', ':', 'dirname']
                j = i - 1
                while j > lexem_start:
                    if source[j] == ':' and source[j-1] != '@':
                        i = j
                        break
                    j -= 1

                if source[i:i+1] == '/' and source[i-1:i] in 'IЦ':
                    if source[i-2:i-1] == ' ':
                        category = Token.Category.OPERATOR
                    else:
                        raise Error('please clarify your intention by putting space character before or after `I`', i-1)

                elif source[i:i+1] == "'": # this is a named argument, a raw string or a hexadecimal number
                    i += 1
                    if source[i:i+1] in (' ', "\n"): # this is a named argument
                        category = Token.Category.NAME
                    elif source[i:i+1] in ('‘', "'"): # ’ # this is a raw string
                        i -= 1
                        category = Token.Category.NAME
                    else: # this is a hexadecimal number
                        while i < len(source) and (is_hexadecimal_digit(source[i]) or source[i] == "'"):
                            i += 1
                        if not (source[lexem_start+4:lexem_start+5] == "'" or source[i-3:i-2] == "'" or source[i-2:i-1] == "'"):
                            raise Error('digit separator in this hexadecimal number is located in the wrong place', lexem_start)
                        category = Token.Category.NUMERIC_LITERAL

                elif source[lexem_start:i] in keywords:
                    if source[lexem_start:i] in ('V', 'П', 'var', 'пер'): # it is more convenient to consider V/var as [type] name, not a keyword
                        category = Token.Category.NAME
                        if source[i:i+1] == '&':
                            i += 1
                    elif source[lexem_start:i] in ('N', 'Н', 'null', 'нуль'):
                        category = Token.Category.CONSTANT
                    else:
                        category = Token.Category.KEYWORD
                        if source[i:i+1] == '.': # this is composite keyword like `L.break`
                            i += 1
                            while i < len(source) and (source[i].isalpha() or source[i] in '_.'):
                                i += 1
                            if source[lexem_start:i] in ('L.index', 'Ц.индекс', 'loop.index', 'цикл.индекс'): # for correct STRING_CONCATENATOR insertion
                                category = Token.Category.NAME

                elif source[lexem_start:i+1] == 'f:': # this is a f-string
                    i += 1
                    if source[i] in ('‘', "'"): # ’
                        while i < len(source) and source[i] == "'":
                            i += 1
                        if source[i:i+1] != '‘': # ’
                            raise Error('expected left single quotation mark', i)
                        startqpos = i
                        i += 1
                        nesting_level = 1
                        while True:
                            if i == len(source):
                                raise Error('unpaired left single quotation mark', startqpos)
                            ch = source[i]
                            i += 1
                            if ch == "‘":
                                nesting_level += 1
                            elif ch == "’":
                                nesting_level -= 1
                                if nesting_level == 0:
                                    break
                        while i < len(source) and source[i] == "'":
                            i += 1
                    else:
                        assert(source[i] == '"')
                        startqpos = i
                        i += 1
                        while True:
                            if i == len(source):
                                raise Error('unclosed string literal', startqpos)
                            ch = source[i]
                            i += 1
                            if ch == '\\':
                                if i == len(source):
                                    continue
                                i += 1
                            elif ch == '"':
                                break
                    tokens.append(Token(lexem_start, lexem_start + 3, Token.Category.FSTRING))
                    j = lexem_start + 3
                    substr_start = j
                    while j < i - 1:
                        if source[j] == '{':
                            if source[j+1] == '{':
                                j += 2
                                continue
                            if j > substr_start:
                                tokens.append(Token(substr_start, j, Token.Category.STRING_LITERAL))
                            tokens.append(Token(j, j, Token.Category.SCOPE_BEGIN))
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
                                elif source[j] == ':' and nesting_level == 0 and (source[j+1] in ('<', '.', ' ') or source[j+1].isdigit()):
                                    colon_pos = j
                                j += 1
                            try:
                                for new_token in tokenize(source[s:colon_pos if colon_pos is not None else j]):
                                    tokens.append(Token(new_token.start + s, new_token.end + s, new_token.category))
                            except Error as error:
                                raise Error(error.message, error.pos + s)
                            if colon_pos is not None:
                                #tokens.append(Token(colon_pos, colon_pos, Token.Category.STATEMENT_SEPARATOR))
                                #tokens.append(Token(colon_pos + 1, j, Token.Category.STRING_LITERAL))
                                tokens.append(Token(colon_pos + 1, j, Token.Category.STATEMENT_SEPARATOR))
                            tokens.append(Token(j, j, Token.Category.SCOPE_END))
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

                else:
                    category = Token.Category.NAME

            elif '0' <= ch <= '9' or (ch == '.' and '0' <= source[i:i+1] <= '9'): # this is NUMERIC_LITERAL or CONSTANT 0B or 1B
                if ch in '01' and source[i:i+1] in ('B', 'В') and not (is_hexadecimal_digit(source[i+1:i+2]) or source[i+1:i+2] == "'"):
                    i += 1
                    category = Token.Category.CONSTANT
                else:
                    is_hex = False
                    while i < len(source) and is_hexadecimal_digit(source[i]):
                        if not ('0' <= source[i] <= '9'):
                            if source[i] in 'eE' and source[i+1:i+2] in ('-', '+'): # fix `1e-10`
                                break
                            is_hex = True
                        i += 1

                    next_digit_separator = 0
                    is_oct_or_bin = False
                    if i < len(source) and source[i] == "'":
                        if i - lexem_start in (2, 1): # special handling for 12'345/1'234 (чтобы это не считалось short/ultrashort hexadecimal number)
                            j = i + 1
                            while j < len(source) and is_hexadecimal_digit(source[j]):
                                if not ('0' <= source[j] <= '9'):
                                    is_hex = True
                                j += 1
                            next_digit_separator = j - 1 - i
                        elif i - lexem_start == 4: # special handling for 1010'1111b (чтобы это не считалось hexadecimal number)
                            j = i + 1
                            while j < len(source) and ((is_hexadecimal_digit(source[j]) and not source[j] in 'bд') or source[j] == "'"): # I know, checking for `in 'bд'` is hacky
                                j += 1
                            if j < len(source) and source[j] in 'oоbд':
                                is_oct_or_bin = True

                    if i < len(source) and source[i] == "'" and ((i - lexem_start == 4 and not is_oct_or_bin) or (i - lexem_start in (2, 1) and (next_digit_separator != 3 or is_hex))): # this is a hexadecimal number
                        if i - lexem_start == 2: # this is a short hexadecimal number
                            while True:
                                i += 1
                                if i + 2 > len(source) or not is_hexadecimal_digit(source[i]) or not is_hexadecimal_digit(source[i+1]):
                                    raise Error('wrong short hexadecimal number', lexem_start)
                                i += 2
                                if i < len(source) and is_hexadecimal_digit(source[i]):
                                    raise Error('expected end of short hexadecimal number', i)
                                if source[i:i+1] != "'":
                                    break
                        elif i - lexem_start == 1: # this is an ultrashort hexadecimal number
                            i += 1
                            if i + 1 > len(source) or not is_hexadecimal_digit(source[i]):
                                raise Error('wrong ultrashort hexadecimal number', lexem_start)
                            i += 1
                            if i < len(source) and is_hexadecimal_digit(source[i]):
                                raise Error('expected end of ultrashort hexadecimal number', i)
                        else:
                            i += 1
                            while i < len(source) and is_hexadecimal_digit(source[i]):
                                i += 1
                                if (i - lexem_start) % 5 == 4 and i < len(source):
                                    if source[i] != "'":
                                        if not is_hexadecimal_digit(source[i]):
                                            break
                                        raise Error('here should be a digit separator in hexadecimal number', i)
                                    i += 1
                            if i < len(source) and source[i] == "'":
                                raise Error('digit separator in hexadecimal number is located in the wrong place', i)
                            if (i - lexem_start) % 5 != 4:
                                raise Error('after this digit separator there should be 4 digits in hexadecimal number', source.rfind("'", 0, i))
                    else:
                        while i < len(source) and ('0' <= source[i] <= '9' or source[i] in "'.eE"):
                            if source[i:i+2] in ('..', '.<', '.+'):
                                break
                            if source[i] in 'eE':
                                if source[i+1:i+2] in '-+':
                                    i += 1
                            i += 1
                        if source[i:i+1] in ('o', 'о', 'b', 'д', 's', 'i'):
                            i += 1
                        elif "'" in source[lexem_start:i] and not '.' in source[lexem_start:i]: # float numbers do not checked for a while
                            number = source[lexem_start:i].replace("'", '')
                            number_with_separators = ''
                            j = len(number)
                            while j > 3:
                                number_with_separators = "'" + number[j-3:j] + number_with_separators
                                j -= 3
                            number_with_separators = number[0:j] + number_with_separators
                            if source[lexem_start:i] != number_with_separators:
                                raise Error('digit separator in this number is located in the wrong place (should be: '+ number_with_separators +')', lexem_start)
                    category = Token.Category.NUMERIC_LITERAL

            elif ch == "'" and source[i:i+1] == ',': # this is a named-only arguments mark
                i += 1
                category = Token.Category.DELIMITER

            elif ch == '"':
                if source[i] == '"' \
                        and tokens[-1].category == Token.Category.STRING_CONCATENATOR \
                        and tokens[-2].category == Token.Category.STRING_LITERAL \
                        and tokens[-2].value(source)[0] == '‘': # ’ // for cases like r = abc‘some big ...’""
                    i += 1                                      #   \\                   ‘... string’
                    continue # [(
                startqpos = i - 1
                if len(tokens) and tokens[-1].end == startqpos and ((tokens[-1].category == Token.Category.NAME and tokens[-1].value(source)[-1] != "'") or tokens[-1].value(source) in (')', ']')):
                    tokens.append(Token(lexem_start, lexem_start, Token.Category.STRING_CONCATENATOR))
                while True:
                    if i == len(source):
                        raise Error('unclosed string literal', startqpos)
                    ch = source[i]
                    i += 1
                    if ch == '\\':
                        if i == len(source):
                            continue
                        i += 1
                    elif ch == '"':
                        break
                if source[i:i+1].isalpha() or source[i:i+1] in ('_', '@', ':', '‘', '('): # )’
                    tokens.append(Token(lexem_start, i, Token.Category.STRING_LITERAL))
                    tokens.append(Token(i, i, Token.Category.STRING_CONCATENATOR))
                    continue
                category = Token.Category.STRING_LITERAL

            elif ch in "‘'" or (ch == '|' and source[i:i+1] in ('‘', "'")): # ’
                if source[i] == '’' \
                        and tokens[-1].category == Token.Category.STRING_CONCATENATOR \
                        and tokens[-2].category == Token.Category.STRING_LITERAL \
                        and tokens[-2].value(source)[0] == '"': # // for cases like r = abc"some big ..."‘’
                    i += 1                                      # \\                   ‘... string’
                    continue # ‘[(
                if len(tokens) and tokens[-1].end == i - 1 and ((tokens[-1].category == Token.Category.NAME and tokens[-1].value(source)[-1] != "'") or tokens[-1].value(source) in (')', ']')):
                    tokens.append(Token(lexem_start, lexem_start, Token.Category.STRING_CONCATENATOR))
                    if source[i] == '’': # for cases like `a‘’b`
                        i += 1
                        continue
                if ch != '|':
                    i -= 1
                while i < len(source) and source[i] == "'":
                    i += 1
                if source[i:i+1] != '‘': # ’
                    raise Error('expected left single quotation mark', i)
                startqpos = i
                i += 1
                nesting_level = 1
                while True:
                    if i == len(source):
                        raise Error('unpaired left single quotation mark', startqpos)
                    ch = source[i]
                    i += 1
                    if ch == "‘":
                        nesting_level += 1
                    elif ch == "’":
                        nesting_level -= 1
                        if nesting_level == 0:
                            break
                while i < len(source) and source[i] == "'":
                    i += 1
                if source[i:i+1].isalpha() or source[i:i+1] in ('_', '@', ':', '"', '('): # )
                    tokens.append(Token(lexem_start, i, Token.Category.STRING_LITERAL))
                    tokens.append(Token(i, i, Token.Category.STRING_CONCATENATOR))
                    continue
                category = Token.Category.STRING_LITERAL

            elif ch == '{':
                indentation_levels.append((-1, True))
                nesting_elements.append((Char('{'), lexem_start)) # }
                category = Token.Category.SCOPE_BEGIN
            elif ch == '}':
                if len(nesting_elements) == 0 or nesting_elements[-1][0] != '{':
                    raise Error('there is no corresponding opening brace for `}`', lexem_start)
                nesting_elements.pop()
                while indentation_levels[-1][1] != True:
                    tokens.append(Token(lexem_start, lexem_start, Token.Category.SCOPE_END))
                    if implied_scopes is not None: # {
                        implied_scopes.append((Char('}'), lexem_start))
                    indentation_levels.pop()
                assert(indentation_levels.pop()[1] == True)
                category = Token.Category.SCOPE_END
            elif ch == ';':
                category = Token.Category.STATEMENT_SEPARATOR

            elif ch in (',', '.', ':'):
                category = Token.Category.DELIMITER
                if ch == '.' and i < len(source) and source[i] == ':': # for `.:`
                    i += 1

            elif ch in '([':
                if source[lexem_start:lexem_start+3] == '(.)':
                    i += 2
                    category = Token.Category.NAME
                else:
                    if ch == '[' and source[i] == '%': # ]
                        i += 1
                    nesting_elements.append((ch, lexem_start))
                    category = Token.Category.DELIMITER
            elif ch in '])': # ([
                if len(nesting_elements) == 0 or nesting_elements[-1][0] != {']':'[', ')':'('}[ch]: # ])
                    raise Error('there is no corresponding opening parenthesis/bracket for `' + ch + '`', lexem_start)
                nesting_elements.pop()
                category = Token.Category.DELIMITER

            else:
                raise Error('unexpected character `' + ch + '`', lexem_start)

            tokens.append(Token(lexem_start, i, category))

    if len(nesting_elements):
        raise Error('there is no corresponding closing parenthesis/bracket/brace for `' + nesting_elements[-1][0] + '`', nesting_elements[-1][1])

    # [4:] [-1]:‘At the end of the file, a DEDENT token is generated for each number remaining on the stack that is larger than zero.’
    while len(indentation_levels):
        assert(indentation_levels[-1][1] != True)
        tokens.append(Token(i, i, Token.Category.SCOPE_END))
        if implied_scopes is not None: # {
            implied_scopes.append((Char('}'), i-1 if source[-1] == "\n" else i))
        indentation_levels.pop()

    return tokens
