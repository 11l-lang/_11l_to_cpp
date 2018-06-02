"""
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
from enum import Enum

keywords = ['else', 'fn', 'if', 'in', 'loop', 'null', 'result', 'return', 'switch', 'type', 'typeof']
# new_scope_keywords = ['else', 'fn', 'if', 'loop', 'switch', 'type']
# Решил отказаться от учёта new_scope_keywords на уровне лексического анализатора из-за loop.break и case в switch
binary_operators = [[], ['+', '-', '*', '/', '&', '|'], ['+=', '-=', '*=', '/=', '&&', '||'], ['<<=', '>>=']]
binary_operators[1].remove('-') # Решил просто не считать `-` за бинарный оператор в контексте автоматического склеивания строк, так как `-` к тому же ещё и модификатор константности
unary_operators = [[], [], ['++', '--'], []]


class Exception(Exception):
    def __init__(self, message, pos):
        self.message = message
        self.pos = pos

class Token:
    class Category(Enum):
        IDENTIFIER = 0
        KEYWORD = 1
        DELIMITER = 2 # SEPARATOR = 2
        OPERATOR = 3
        NUMERIC_LITERAL = 4
        STRING_LITERAL = 5
        SCOPE_BEGIN = 6 # similar to ‘INDENT token in Python’[https://docs.python.org/3/reference/lexical_analysis.html][-1]
        SCOPE_END = 7   # similar to ‘DEDENT token in Python’[-1]
        STATEMENT_SEPARATOR = 8

    def __init__(self, start, end, category):
        self.start = start
        self.end = end
        self.category = category

def tokenize(source, implied_scopes = None, line_continuations = None, newline_chars = None, comments = None):
    tokens = []
    indentation_levels = []
    nesting_elements = [] # логически этот стек можно объединить с indentation_levels, но так немного удобнее (например для обработки [./tests.txt]:‘// But you can close parenthesis/bracket’, конкретно: для проверок `nesting_elements[-1][0] != ...`)
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

            if source[i] in "\r\n" or source[i:i+2] == '//': # lines with only whitespace and/or comments do not affect the indentation
                continue

            if source[i] in "{}": # Indentation level of lines starting with { or } is ignored
                continue

            if len(tokens) \
               and tokens[-1].category == Token.Category.OPERATOR \
               and source[tokens[-1].start:tokens[-1].end] in binary_operators[tokens[-1].end - tokens[-1].start]: # ‘Every line of code which ends with any binary operator should be joined with the following line of code.’:[https://github.com/JuliaLang/julia/issues/2097#issuecomment-339924750][-339924750]<
                if line_continuations != None:
                    line_continuations.append(tokens[-1].end)
                continue

            # if not (len(indentation_levels) and indentation_levels[-1][0] == None): # сразу после символа `{` это [:правило] не действует ...а хотя не могу подобрать пример, который бы показывал необходимость такой проверки, а потому оставлю этот if закомментированным # }
            if ((source[i    ] in binary_operators[1]
              or source[i:i+2] in binary_operators[2]
              or source[i:i+3] in binary_operators[3]) # [правило:] ‘Every line of code which begins with any binary operator should be joined with the previous line of code.’:[-339924750]<
              and not (source[i    ] in unary_operators[1]  # Rude fix for:
                    or source[i:i+2] in unary_operators[2]  # a=b
                    or source[i:i+3] in unary_operators[3]) # ++i // Plus symbol at the beginning here should not be treated as binary + operator, so there is no implied line joining
              and (source[i] != '&' or source[i+1:i+2] == ' ')): # Символ `&` обрабатывается по-особенному — склеивание строк происходит только если после него стоит пробел
                if len(tokens) == 0:
                    raise Exception('source can not starts with a binary operator', i)
                if line_continuations != None:
                    line_continuations.append(tokens[-1].end)
                continue

            if tabs and spaces:
                next_line_pos = source.find("\n", i)
                raise Exception('mixing tabs and spaces in indentation: `' + source[linestart:i].replace(' ', 'S').replace("\t", 'TAB') + source[i:next_line_pos if next_line_pos != -1 else len(source)] + '`', i)

            indentation_level = i - linestart
            if len(indentation_levels) and indentation_levels[-1][0] == None: # сразу после символа `{` идёт новый произвольный отступ (понижение уровня отступа может быть полезно, если вдруг отступ оказался слишком большой), который действует вплоть до парного символа `}`
                indentation_levels[-1][0] = indentation_level # || maybe this is unnecessary (actually it is necessary, see test "fn f()\n{\na = 1")
                # // This is uncertain piece of code:
                indentation_tabs = tabs
            else:
                if indentation_level > 0 and len(indentation_levels) and prev_indentation_level > 0 and indentation_tabs != tabs:
                    e = i + 1
                    while e < len(source) and source[e] not in "\r\n":
                        e += 1
                    raise Exception("inconsistent indentations: ```\n" + prev_indentation_level*('TAB' if indentation_tabs else 'S') + source[prev_linestart:linestart]
                        + (i-linestart)*('TAB' if tabs else 'S') + source[i:e] + "\n```", i)
                prev_linestart = i

                prev_indentation_level = indentation_levels[-1][0] if len(indentation_levels) else 0

                if indentation_level == prev_indentation_level: # [1:] [-1]:‘If it is equal, nothing happens.’ :)(: [:2]
                    if len(tokens):
                        tokens.append(Token(linestart-1, linestart, Token.Category.STATEMENT_SEPARATOR))
                elif indentation_level > prev_indentation_level: # [2:] [-1]:‘If it is larger, it is pushed on the stack, and one INDENT token is generated.’ [:3]
                    if prev_indentation_level == 0: # len(indentation_levels) == 0 or indentation_levels[-1][0] == 0:
                        indentation_tabs = tabs # первоначальная/новая установка символа для отступа (либо табуляция, либо пробелы) производится только от нулевого уровня отступа
                    indentation_levels.append([indentation_level, False])
                    tokens.append(Token(i, i, Token.Category.SCOPE_BEGIN))
                    if implied_scopes != None:
                        implied_scopes.append(('{', tokens[-2].end + (1 if source[tokens[-2].end] in " \n" else 0)))
                else: # [3:] [-1]:‘If it is smaller, it ~‘must’ be one of the numbers occurring on the stack; all numbers on the stack that are larger are popped off, and for each number popped off a DEDENT token is generated.’ [:4]
                    while True:
                        if indentation_levels[-1][1]:
                            raise Exception('too much unindent, what is this unindent intended for?', i)
                        indentation_levels.pop()
                        tokens.append(Token(i, i, Token.Category.SCOPE_END))
                        if implied_scopes != None:
                            implied_scopes.append(('}', i))
                        level, explicit_scope_via_curly_braces = indentation_levels[-1] if len(indentation_levels) else [0, False]
                        if level == indentation_level:
                            break
                        if level < indentation_level:
                            raise Exception('unindent does not match any outer indentation level', i)

                prev_indentation_level = indentation_level

        ch = source[i]

        if ch in " \t":
            i += 1 # just skip whitespace characters
        elif ch in "\r\n":
            if newline_chars != None:
                newline_chars.append(i)
            i += 1
            if ch == "\r" and source[i:i+1] == "\n":
                i += 1
            if len(nesting_elements) == 0 or nesting_elements[-1][0] not in '([': # если мы внутри скобок, то начинать новую строку не нужно # ])
                begin_of_line = True
        elif ch == '/' and source[i+1:i+2] == '/':
            i += 2
            comment_start = i
            while i < len(source) and source[i] not in "\r\n":
                i += 1
            if comments != None:
                comments.append((comment_start, i))
        else:
            operator = None
            for op in ['<<', '>>', '<=', '>=', '==', '!=', '+=', '-=', '*=', '/=', '%', '&=', '|=', '^=', '++', '--', '+', '-', '*', '/', '%', '!', '&', '|', '^', '~', '<', '>', '=']:
                if source[i:i+len(op)] == op:
                    operator = op
                    break

            lexem_start = i
            i += 1

            if operator:
                i = lexem_start + len(operator)
                category = Token.Category.OPERATOR
            elif 'a' <= ch <= 'z' or 'A' <= ch <= 'Z' or ch == '_': # this is IDENTIFIER or KEYWORD
                while i < len(source):
                    ch = source[i]
                    if not ('a' <= ch <= 'z' or 'A' <= ch <= 'Z' or ch == '_' or '0' <= ch <= '9' or ch == '?'):
                        break
                    i += 1
                if source[i:i+1] == '/' and source[i-1:i] in 'IЦ':
                    if source[i-2:i-1] == ' ':
                        category = Token.Category.OPERATOR
                    else:
                        raise Exception('please clarify your intention by putting space character before or after `I`', i-1)
                elif source[lexem_start:i] in keywords:
                    category = Token.Category.KEYWORD
                else:
                    category = Token.Category.IDENTIFIER

            elif '0' <= ch <= '9': # this is NUMERIC_LITERAL
                if ch in '01' and source[i:i+1] == 'B':
                    i += 1
                else:
                    while i < len(source) and '0' <= source[i] <= '9':
                        i += 1
                category = Token.Category.NUMERIC_LITERAL

            elif ch == '"':
                startqpos = i - 1
                while True:
                    if i == len(source):
                        raise Exception('unclosed string literal', startqpos)
                    ch = source[i]
                    i += 1
                    if ch == '"':
                        break
                category = Token.Category.STRING_LITERAL

            elif ch == '‘':
                startqpos = i - 1
                nesting_level = 1
                while True:
                    if i == len(source):
                        raise Exception('unpaired left single quotation mark', startqpos)
                    ch = source[i]
                    i += 1
                    if ch == "‘":
                        nesting_level += 1
                    elif ch == "’":
                        nesting_level -= 1
                        if nesting_level == 0:
                            break
                category = Token.Category.STRING_LITERAL

            elif ch == '{':
                indentation_levels.append([None, True])
                nesting_elements.append(('{', lexem_start)) # }
                category = Token.Category.SCOPE_BEGIN
            elif ch == '}':
                if len(nesting_elements) == 0 or nesting_elements[-1][0] != '{':
                    raise Exception('there is no corresponding opening brace for `}`', lexem_start)
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

            elif ch in [',', '.', ':', '@']:
                category = Token.Category.DELIMITER

            elif ch in '([':
                nesting_elements.append((ch, lexem_start))
                category = Token.Category.DELIMITER
            elif ch in '])':
                if len(nesting_elements) == 0 or nesting_elements[-1][0] != {']':'[', ')':'('}[ch]: # ])
                    raise Exception('there is no corresponding opening parenthesis/bracket for `' + ch + '`', lexem_start)
                nesting_elements.pop()
                #end_scope(lexem_start)
                category = Token.Category.DELIMITER

            else:
                raise Exception('unexpected character ' + ch, lexem_start)

            tokens.append(Token(lexem_start, i, category))

    if len(nesting_elements):
        raise Exception('there is no corresponding closing parenthesis/bracket/brace for `' + nesting_elements[-1][0] + '`', nesting_elements[-1][1])

    #end_scope() # [4:] [-1]:‘At the end of the file, a DEDENT token is generated for each number remaining on the stack that is larger than zero.’
    while len(indentation_levels) and indentation_levels[-1][1] != True:
        tokens.append(Token(i, i, Token.Category.SCOPE_END))
        if implied_scopes != None: # {
            implied_scopes.append(('}', i-1 if source[-1] == "\n" else i))
        indentation_levels.pop()

    return tokens
