R"""
После данной обработки отступы перестают играть роль — границу `scope` всегда определяют фигурные скобки.
Также здесь выполняется склеивание строк, и таким образом границу statement\утверждения задаёт либо символ `;`,
либо символ новой строки (при условии, что перед ним не стоит символ `…`!).

===============================================================================================================
Ошибки:
---------------------------------------------------------------------------------------------------------------
Error: `if/else/fn/loop/switch/type` scope is empty.
---------------------------------------------------------------------------------------------------------------
Существуют операторы, которые всегда требуют нового scope\блока, который можно обозначить двумя способами:
1. Начать следующую строку с отступом относительно предыдущей, например:
if condition\условие
    scope\блок
2. Заключить блок\scope в фигурные скобки:
if condition\условие {scope\блок}

Примечание. При использовании второго способа блок\scope может иметь произвольный уровень отступа:
if condition\условие
{
scope\блок
}

---------------------------------------------------------------------------------------------------------------
Error: `if/else/fn/loop/switch/type` scope is empty, after applied implied line joining: ```...```
---------------------------------------------------------------------------------------------------------------
Сообщение об ошибке аналогично предыдущему, но выделено в отдельное сообщение об ошибке, так как может
возникать по вине ошибочного срабатывания автоматического склеивания строк (и показывается оно тогда, когда
было произведено склеивание строк в месте данной ошибки).

---------------------------------------------------------------------------------------------------------------
Error: mixing tabs and spaces in indentation: `...`
---------------------------------------------------------------------------------------------------------------
В одной строке для отступа используется смесь пробелов и символов табуляции.
Выберите что-либо одно (желательно сразу для всего файла): либо пробелы для отступа, либо табуляцию.
Примечание: внутри строковых литералов, а также внутри строк кода можно смешивать пробелы и табуляцию. Эта
ошибка генерируется только при проверке отступа (отступ — последовательность символов пробелов или табуляции от
самого начала строки до первого символа отличного от пробела и табуляции).

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

---------------------------------------------------------------------------------------------------------------
Error: unindent does not match any outer indentation level
---------------------------------------------------------------------------------------------------------------
[-Добавить описание ошибки.-]
===============================================================================================================
"""
from enum import IntEnum
from typing import List

keywords = ['A',     'C',  'I',    'E',     'F',  'L',    'N',    'R',       'S',       'T',    'X',
            'А',     'С',  'Е',    'И',     'Ф',  'Ц',    'Н',    'Р',       'В',       'Т',    'Х',
            'var',   'in', 'if',   'else',  'fn', 'loop', 'null', 'return',  'switch',  'type', 'exception',
            'перем', 'С',  'если', 'иначе', 'фн', 'цикл', 'нуль', 'вернуть', 'выбрать', 'тип',  'исключение']
#keywords.remove('C'); keywords.remove('С'); keywords.remove('in') # it is more convenient to consider C/in as an operator, not a keyword (however, this line is not necessary)
# new_scope_keywords = ['else', 'fn', 'if', 'loop', 'switch', 'type']
# Решил отказаться от учёта new_scope_keywords на уровне лексического анализатора из-за loop.break и case в switch
binary_operators : List[List[str]] = [[], ['+', '-', '*', '/', '%', '^', '&', '|', '<', '>', '='], ['<<', '>>', '<=', '>=', '==', '!=', '+=', '-=', '*=', '/=', '%=', '&&', '||', '&=', '|=', '^=', '->', '..', '.<', '<.', 'I/', 'Ц/', 'C ', 'С '], ['<<=', '>>=', '‘’=', '[+]', '[&]', '[|]', '(+)', '<.<', 'I/=', 'Ц/=', 'in ', '!C ', '!С '], ['[+]=', '[&]=', '[|]=', '(+)=', '!in ']]
unary_operators = [[], ['!'], ['++', '--'], ['(-)']]
sorted_operators = sorted(binary_operators[1] + binary_operators[2] + binary_operators[3] + binary_operators[4] + unary_operators[1] + unary_operators[2] + unary_operators[3], key = lambda x: len(x), reverse = True)
binary_operators[1].remove('-') # Решил просто не считать `-` за бинарный оператор в контексте автоматического склеивания строк, так как `-` к тому же ещё и квалификатор константности


class Error(Exception):
    def __init__(self, message, pos):
        self.message = message
        self.pos = pos

class Token:
    class Category(IntEnum):
        NAME = 0 # or IDENTIFIER
        KEYWORD = 1
        CONSTANT = 2
        DELIMITER = 3 # SEPARATOR = 3
        OPERATOR = 4
        NUMERIC_LITERAL = 5
        STRING_LITERAL = 6
        STRING_CONCATENATOR = 7 # special token inserted between adjacent string literal and some identifier
        SCOPE_BEGIN = 8 # similar to ‘INDENT token in Python’[https://docs.python.org/3/reference/lexical_analysis.html][-1]
        SCOPE_END   = 9 # similar to ‘DEDENT token in Python’[-1]
        STATEMENT_SEPARATOR = 10

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

def tokenize(source, implied_scopes = None, line_continuations = None, comments = None):
    tokens = []
    indentation_levels = []
    nesting_elements = [] # логически этот стек можно объединить с indentation_levels, но так немного удобнее (конкретно: для проверок `nesting_elements[-1][0] != ...`)
    i = 0
    #def end_scope(opt = 0):
    #    pass

    begin_of_line = True
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

            if source[i] in "\r\n" or source[i:i+2] in ('//', '\\\\', R'\‘', R'\(', R'\{', R'\['): # ]})’ # lines with only whitespace and/or comments do not affect the indentation
                continue

            if source[i] in "{}": # Indentation level of lines starting with { or } is ignored
                continue

            if len(tokens) \
               and tokens[-1].category == Token.Category.STRING_CONCATENATOR \
               and source[i] in '"\'‘': # ’ and not source[i+1:i+2] in ({'"':'"', '‘':'’'}[source[i]],):
                if line_continuations != None:
                    line_continuations.append(tokens[-1].end)
                if source[i:i+2] in ('""', '‘’'):
                    i += 2
                continue

            if len(tokens) \
               and tokens[-1].category == Token.Category.STRING_LITERAL \
               and source[i:i+2] in ('""', '‘’'):
                if line_continuations != None:
                    line_continuations.append(tokens[-1].end)
                tokens.append(Token(i, i, Token.Category.STRING_CONCATENATOR))
                i += 2
                continue

            if len(tokens) \
               and tokens[-1].category == Token.Category.OPERATOR \
               and tokens[-1].value(source) in binary_operators[tokens[-1].end - tokens[-1].start]: # ‘Every line of code which ends with any binary operator should be joined with the following line of code.’:[https://github.com/JuliaLang/julia/issues/2097#issuecomment-339924750][-339924750]<
                if line_continuations != None:
                    line_continuations.append(tokens[-1].end)
                continue

            # if not (len(indentation_levels) and indentation_levels[-1][0] == None): # сразу после символа `{` это [:правило] не действует ...а хотя не могу подобрать пример, который бы показывал необходимость такой проверки, а потому оставлю этот if закомментированным # }
            if ((source[i    ] in binary_operators[1]
              or source[i:i+2] in binary_operators[2]
              or source[i:i+3] in binary_operators[3]
              or source[i:i+4] in binary_operators[4]) # [правило:] ‘Every line of code which begins with any binary operator should be joined with the previous line of code.’:[-339924750]<
              and not (source[i    ] in unary_operators[1]  # Rude fix for:
                    or source[i:i+2] in unary_operators[2]  # a=b
                    or source[i:i+3] in unary_operators[3]) # ++i // Plus symbol at the beginning here should not be treated as binary + operator, so there is no implied line joining
              and (source[i] != '&' or source[i+1:i+2] == ' ')): # Символ `&` обрабатывается по-особенному — склеивание строк происходит только если после него стоит пробел
                if len(tokens) == 0:
                    raise Error('source can not starts with a binary operator', i)
                if line_continuations != None:
                    line_continuations.append(tokens[-1].end)
                continue

            if source[i:i+2] == R'\.': # // Support for constructions like: ||| You need just to add `\` at the each line starting from dot:
                if len(tokens):        # \\ result = abc.method1()          ||| result = abc.method1()
                    i += 1             # \\     .method2()                  |||     \.method2()
               #else: # with `if len(tokens): i += 1` there is no need for this else branch
               #    raise Error('unexpected character `\`')
                    if line_continuations != None:
                        line_continuations.append(tokens[-1].end)
                continue

            if tabs and spaces:
                next_line_pos = source.find("\n", i)
                raise Error('mixing tabs and spaces in indentation: `' + source[linestart:i].replace(' ', 'S').replace("\t", 'TAB') + source[i:next_line_pos if next_line_pos != -1 else len(source)] + '`', i)

            indentation_level = i - linestart
            if len(indentation_levels) and indentation_levels[-1][0] == None: # сразу после символа `{` идёт новый произвольный отступ (понижение уровня отступа может быть полезно, если вдруг отступ оказался слишком большой), который действует вплоть до парного символа `}`
                indentation_levels[-1][0] = indentation_level # || maybe this is unnecessary (actually it is necessary, see test "fn f()\n{\na = 1") // }
                # // This is uncertain piece of code:
                indentation_tabs = tabs
            else:
                if indentation_level > 0 and len(indentation_levels) and prev_indentation_level > 0 and indentation_tabs != tabs:
                    e = i + 1
                    while e < len(source) and source[e] not in "\r\n":
                        e += 1
                    raise Error("inconsistent indentations:\n```\n" + prev_indentation_level*('TAB' if indentation_tabs else 'S') + source[prev_linestart:linestart]
                        + (i-linestart)*('TAB' if tabs else 'S') + source[i:e] + "\n```", i)
                prev_linestart = i

                prev_indentation_level = indentation_levels[-1][0] if len(indentation_levels) else 0

                if indentation_level == prev_indentation_level: # [1:] [-1]:‘If it is equal, nothing happens.’ :)(: [:2]
                    if len(tokens):# and tokens[-1].category != Token.Category.SCOPE_END:
                        tokens.append(Token(linestart-1, linestart, Token.Category.STATEMENT_SEPARATOR))
                elif indentation_level > prev_indentation_level: # [2:] [-1]:‘If it is larger, it is pushed on the stack, and one INDENT token is generated.’ [:3]
                    if prev_indentation_level == 0: # len(indentation_levels) == 0 or indentation_levels[-1][0] == 0:
                        indentation_tabs = tabs # первоначальная/новая установка символа для отступа (либо табуляция, либо пробелы) производится только от нулевого уровня отступа
                    indentation_levels.append([indentation_level, False])
                    tokens.append(Token(linestart, i, Token.Category.SCOPE_BEGIN))
                    if implied_scopes != None:
                        implied_scopes.append(('{', tokens[-2].end + (1 if source[tokens[-2].end] in " \n" else 0)))
                else: # [3:] [-1]:‘If it is smaller, it ~‘must’ be one of the numbers occurring on the stack; all numbers on the stack that are larger are popped off, and for each number popped off a DEDENT token is generated.’ [:4]
                    while True:
                        if indentation_levels[-1][1]:
                            raise Error('too much unindent, what is this unindent intended for?', i)
                        indentation_levels.pop()
                        tokens.append(Token(i, i, Token.Category.SCOPE_END))
                        if implied_scopes != None:
                            implied_scopes.append(('}', i))
                        level, explicit_scope_via_curly_braces = indentation_levels[-1] if len(indentation_levels) else [0, False]
                        if level == indentation_level:
                            break
                        if level < indentation_level:
                            raise Error('unindent does not match any outer indentation level', i)

                prev_indentation_level = indentation_level

        ch = source[i]

        if ch in " \t":
            i += 1 # just skip whitespace characters
        elif ch in "\r\n":
            #if newline_chars != None: # rejected this code as it does not count newline characters inside comments and string literals — better to use pqmarkup.py approach — on demand (i.e. only when error occured) calculation of newline characters array
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
            if comments != None:
                comments.append((comment_start, i))
        elif ch == '\\' and source[i+1:i+2] in "‘({[": # multi-line comment # ]})’
            comment_start = i
            lbr = source[i+1]
            rbr = {"‘": "’", "(": ")", "{": "}", "[": "]"}[lbr]
            i += 2
            nesting_level = 1
            while True:
                ch = source[i]
                if ch == lbr:
                    nesting_level += 1
                elif ch == rbr:
                    nesting_level -= 1
                    if nesting_level == 0:
                        break
                i += 1
                if i == len(source):
                    raise Error('there is no corresponding opening parenthesis/bracket/brace/qoute for `' + lbr + '`', comment_start+1)
            if comments != None:
                comments.append((comment_start, i))
            i += 1
        else:
            def is_hexadecimal_digit(ch):
                return '0' <= ch <= '9' or 'A' <= ch <= 'F' or 'a' <= ch <= 'f' or ch in 'абсдефАБСДЕФ'

            operator = None
            # if ch in 'CС' and not (source[i+1:i+2].isalpha() or source[i+1:i+2].isdigit()): # without this check [and if 'C' is in binary_operators] when identifier starts with `C` (for example `Circle`), then this first letter of identifier is mistakenly considered as an operator
            #     operator = ch
            # else:
            for op in sorted_operators:
                if source[i:i+len(op)] == op:
                    operator = op
                    break

            lexem_start = i
            i += 1

            if operator:
                i = lexem_start + len(operator)
                if source[i-1] == ' ': # for correct handling of operator 'C '/'in ' in external tools (e.g. keyletters_to_keywords.py)
                    i -= 1
                category = Token.Category.OPERATOR
            elif ch.isalpha() or ch in ('_', '@'): # this is NAME/IDENTIFIER or KEYWORD
                while i < len(source) and source[i] == '@':
                    i += 1
                while i < len(source):
                    ch = source[i]
                    if not (ch.isalpha() or ch in '_?:' or '0' <= ch <= '9'):
                        break
                    i += 1
                # Tokenize `fs:path:dirname` to ['fs:path', ':', 'dirname']
                j = i - 1
                while j > lexem_start:
                    if source[j] == ':':
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
                    if source[i:i+1] == ' ': # this is a named argument
                        category = Token.Category.NAME
                    elif source[i:i+1] in ('‘', "'"): # ’ # this is a raw string
                        i -= 1
                        category = Token.Category.NAME
                    else: # this is a hexadecimal number
                        while i < len(source) and (is_hexadecimal_digit(source[i]) or source[i] == "'"):
                            i += 1
                        if not (source[lexem_start+4:lexem_start+5] == "'" or source[i-3:i-2] == "'"):
                            raise Error('digit separator in this hexadecimal number is located in the wrong place', lexem_start)
                        category = Token.Category.NUMERIC_LITERAL

                elif source[lexem_start:i] in keywords:
                    if source[lexem_start:i] in ('A', 'А', 'var', 'перем'): # it is more convenient to consider A/var as [type] name, not a keyword
                        category = Token.Category.NAME
                    elif source[lexem_start:i] in ('N', 'Н', 'null', 'нуль'):
                        category = Token.Category.CONSTANT
                    else:
                        category = Token.Category.KEYWORD
                        if source[i:i+1] == '.': # this is composite keyword like `L.break`
                            i += 1
                            while i < len(source) and (source[i].isalpha() or source[i] in '_.'):
                                i += 1
                else:
                    category = Token.Category.NAME

            elif '0' <= ch <= '9': # this is NUMERIC_LITERAL or CONSTANT 0B or 1B
                if ch in '01' and source[i:i+1] in ('B', 'В') and not (is_hexadecimal_digit(source[i+1:i+2]) or source[i+1:i+2] == "'"):
                    i += 1
                    category = Token.Category.CONSTANT
                else:
                    is_hex = False
                    while i < len(source) and is_hexadecimal_digit(source[i]):
                        if not ('0' <= source[i] <= '9'):
                            is_hex = True
                        i += 1

                    next_digit_separator = 0
                    is_oct_or_bin = False
                    if i < len(source) and source[i] == "'":
                        if i - lexem_start == 2: # special handling for 12'345 (чтобы это не считалось short hexadecimal number)
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

                    if i < len(source) and source[i] == "'" and ((i - lexem_start == 4 and not is_oct_or_bin) or (i - lexem_start == 2 and (next_digit_separator != 3 or is_hex))): # this is a hexadecimal number
                        if i - lexem_start == 2: # this is a short hexadecimal number
                            i += 1
                            if i + 2 > len(source) or not is_hexadecimal_digit(source[i]) or not is_hexadecimal_digit(source[i+1]):
                                raise Error('wrong short hexadecimal number', lexem_start)
                            i += 2
                            if i < len(source) and is_hexadecimal_digit(source[i]):
                                raise Error('expected end of short hexadecimal number', i)
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
                            if source[i:i+2] in ('..', '.<'):
                                break
                            if source[i] in 'eE':
                                if source[i+1:i+2] in '-+':
                                    i += 1
                            i += 1
                        if source[i:i+1] in 'oоbд':
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
                if len(tokens) and ((tokens[-1].category == Token.Category.NAME and tokens[-1].value(source)[-1] != "'") or tokens[-1].value(source) in (')', ']')):
                    tokens.append(Token(lexem_start, lexem_start, Token.Category.STRING_CONCATENATOR))
                startqpos = i - 1
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
                if source[i:i+1].isalpha() or source[i:i+1] in '@‘(': # )’
                    tokens.append(Token(lexem_start, i, Token.Category.STRING_LITERAL))
                    tokens.append(Token(i, i, Token.Category.STRING_CONCATENATOR))
                    continue
                category = Token.Category.STRING_LITERAL

            elif ch in "‘'":
                if source[i] == '’' \
                        and tokens[-1].category == Token.Category.STRING_CONCATENATOR \
                        and tokens[-2].category == Token.Category.STRING_LITERAL \
                        and tokens[-2].value(source)[0] == '"': # // for cases like r = abc"some big ..."‘’
                    i += 1                                      # \\                   ‘... string’
                    continue # ‘[(
                if len(tokens) and ((tokens[-1].category == Token.Category.NAME and tokens[-1].value(source)[-1] != "'") or tokens[-1].value(source) in (')', ']')):
                    tokens.append(Token(lexem_start, lexem_start, Token.Category.STRING_CONCATENATOR))
                    if source[i] == '’': # for cases like `a‘’b`
                        i += 1
                        continue
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
                if source[i:i+1].isalpha() or source[i:i+1] in '@"(': # )
                    tokens.append(Token(lexem_start, i, Token.Category.STRING_LITERAL))
                    tokens.append(Token(i, i, Token.Category.STRING_CONCATENATOR))
                    continue
                category = Token.Category.STRING_LITERAL

            elif ch == '{':
                indentation_levels.append([None, True])
                nesting_elements.append(('{', lexem_start)) # }
                category = Token.Category.SCOPE_BEGIN
            elif ch == '}':
                if len(nesting_elements) == 0 or nesting_elements[-1][0] != '{':
                    raise Error('there is no corresponding opening brace for `}`', lexem_start)
                #end_scope(lexem_start)
                nesting_elements.pop()
                while indentation_levels[-1][1] != True:
                    tokens.append(Token(lexem_start, lexem_start, Token.Category.SCOPE_END))
                    if implied_scopes != None: # {
                        implied_scopes.append(('}', lexem_start))
                    indentation_levels.pop()
                assert(indentation_levels.pop()[1] == True)
                category = Token.Category.SCOPE_END
            elif ch == ';':
                category = Token.Category.STATEMENT_SEPARATOR

            elif ch in (',', '.', ':'):
                category = Token.Category.DELIMITER

            elif ch in '([':
                nesting_elements.append((ch, lexem_start))
                category = Token.Category.DELIMITER
            elif ch in '])': # ([
                if len(nesting_elements) == 0 or nesting_elements[-1][0] != {']':'[', ')':'('}[ch]: # ])
                    raise Error('there is no corresponding opening parenthesis/bracket for `' + ch + '`', lexem_start)
                nesting_elements.pop()
                #end_scope(lexem_start)
                category = Token.Category.DELIMITER

            else:
                raise Error('unexpected character `' + ch + '`', lexem_start)

            tokens.append(Token(lexem_start, i, category))

    if len(nesting_elements):
        raise Error('there is no corresponding closing parenthesis/bracket/brace for `' + nesting_elements[-1][0] + '`', nesting_elements[-1][1])

    #end_scope() # [4:] [-1]:‘At the end of the file, a DEDENT token is generated for each number remaining on the stack that is larger than zero.’
    while len(indentation_levels):
        assert(indentation_levels[-1][1] != True)
        tokens.append(Token(i, i, Token.Category.SCOPE_END))
        if implied_scopes != None: # {
            implied_scopes.append(('}', i-1 if source[-1] == "\n" else i))
        indentation_levels.pop()

    return tokens
