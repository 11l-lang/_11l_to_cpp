from tokenizer import Token
from typing import List, Tuple, Dict, Callable

class Error(Exception):
    def __init__(self, message, pos):
        self.message = message
        self.pos = pos

class Scope:
    parent : 'Scope'
    class Var:
        type : str
        def __init__(self, type):
            self.type = type
    vars : Dict[str, Var]
    is_function : bool

    def __init__(self, func_args):
        self.parent = None
        if func_args != None:
            self.is_function = True
            self.vars = dict(map(lambda x: (x, Scope.Var(None)), func_args))
        else:
            self.is_function = False
            self.vars = {}

    def find_in_current_function(self, name):
        s = self
        while True:
            if name in s.vars:
                return True
            if s.is_function:
                return False
            s = s.parent
            if s == None:
                return False

scope : Scope

class SymbolBase:
    id : str
    lbp : int
    nud_bp : int
    led_bp : int
    nud : Callable[['SymbolNode'], 'SymbolNode']
    led : Callable[['SymbolNode', 'SymbolNode'], 'SymbolNode']

    def set_nud_bp(self, nud_bp, nud):
        self.nud_bp = nud_bp
        self.nud    = nud

    def set_led_bp(self, led_bp, led):
        self.led_bp = led_bp
        self.led    = led

    def __init__(self):
        def nud(s): raise Error('unknown unary operator', s.token.start)
        self.nud = nud
        def led(s, l): raise Error('unknown binary operator', s.token.start)
        self.led = led

class SymbolNode:
    token : Token
    symbol : SymbolBase = None
    children : List['SymbolNode']# = []
    parent : 'SymbolNode' = None
    ast_parent : 'ASTNode'
    function_call : bool = False
    tuple   : bool = False
    is_list : bool = False
    is_dict : bool = False
    is_type : bool = False
    postfix : bool = False
    scope : Scope

    def __init__(self, token):
        self.token = token
        self.children = []
        self.scope = scope

    def append_child(self, child):
        child.parent = self
        self.children.append(child)

    def leftmost(self):
        if self.token.category in (Token.Category.NUMERIC_LITERAL, Token.Category.STRING_LITERAL, Token.Category.NAME, Token.Category.CONSTANT):
            return self.token.start

        if self.symbol.id == '(': # )
            if self.function_call:
                return self.children[0].token.start
            else:
                return self.token.start
        elif self.symbol.id == '[': # ]
            if self.is_list or self.is_dict:
                return self.token.start
            else:
                return self.children[0].token.start

        if len(self.children) in (2, 3):
            return self.children[0].leftmost()

        return self.token.start

    def rightmost(self):
        if self.token.category in (Token.Category.NUMERIC_LITERAL, Token.Category.STRING_LITERAL, Token.Category.NAME, Token.Category.CONSTANT):
            return self.token.end

        if self.symbol.id in '([': # ])
            return self.children[-1].rightmost() + 1

        return self.children[-1].rightmost()

    def to_str(self):
        if self.token.category == Token.Category.NAME:
            return self.token.value(source).lstrip('@')

        if self.token.category == Token.Category.NUMERIC_LITERAL:
            n = self.token.value(source)
            if n[-1] in 'oо':
                return '0' + n[:-1]
            if n[-1] in 'bд':
                return '0b' + n[:-1]
            if n[4:5] == "'" or n[-3:-2] == "'":
                return '0x' + n
            return n

        if self.token.category == Token.Category.STRING_LITERAL:
            if self.token.value(source)[0] == '"':
                return 'u' + self.token.value(source) + '_S'
            return 'u"' + repr(self.token.value(source)[1:-1])[1:-1].replace('"', R'\"').replace(R"\'", "'") + '"_S'

        if self.token.category == Token.Category.CONSTANT:
            return {'N': 'nullptr', 'Н': 'nullptr', '0B': 'false', '0В': 'false', '1B': 'true', '1В': 'true'}[self.token.value(source)]

        def is_char(child):
            return child.token.category == Token.Category.STRING_LITERAL and len(child.token.value(source)) == 3

        def char_or_str(child, is_char):
            if is_char:
                return "u'" + child.token.value(source)[1:-1] + "'_C"
            return child.to_str()

        if self.symbol.id == '(': # )
            if self.function_call:
                func_name = self.children[0].to_str()
                res = func_name + '('
                for i in range(1, len(self.children)):
                    res += self.children[i].to_str()
                    if i < len(self.children)-1:
                        res += ', '
                return res + ')'
            elif self.tuple:
                res = 'make_tuple('
                for i in range(len(self.children)):
                    res += self.children[i].to_str()
                    if i < len(self.children)-1:
                        res += ', '
                if len(self.children) == 1:
                    res += ','
                return res + ')'
            else:
                assert(len(self.children) == 1)
                if self.children[0].symbol.id in ('..', '.<', '<.', '<.<'): # чтобы вместо `(range_el(0, seq.len()))` было `range_el(0, seq.len())`
                    return self.children[0].to_str()
                return '(' + self.children[0].to_str() + ')'

        elif self.symbol.id == '[': # ]
            if self.is_list:
                type_of_values_is_char = True
                for child in self.children:
                    if not is_char(child):
                        type_of_values_is_char = False
                        break
                res = 'create_array({'
                for i in range(len(self.children)):
                    res += char_or_str(self.children[i], type_of_values_is_char)
                    if i < len(self.children)-1:
                        res += ', '
                return res + '})'
            elif self.is_dict:
                char_key = True
                char_val = True
                for child in self.children:
                    assert(child.symbol.id == '=')
                    if not is_char(child.children[0]):
                        char_key = False
                    if not is_char(child.children[1]):
                        char_val = False
                res = 'create_dict(dict_of'
                for child in self.children:
                    res += '(' + char_or_str(child.children[0], char_key) + ', ' + char_or_str(child.children[1], char_val) + ')'
                return res + ')'
            elif self.children[1].token.category == Token.Category.NUMERIC_LITERAL:
                return '_get<' + self.children[1].to_str() + '>(' + self.children[0].to_str() + ')' # for support tuples (e.g. `(1, 2)[0]` -> `_get<0>(make_tuple(1, 2))`)
            else:
                return self.children[0].to_str() + '[' + self.children[1].to_str() + ']'

        elif self.symbol.id in ('S', 'В', 'switch', 'выбрать'):
            char_key = True
            char_val = True
            for i in range(1, len(self.children), 2):
                if not self.children[i].token.value(source) in ('E', 'И', 'else', 'иначе'):
                    if not is_char(self.children[i]):
                        char_key = False
                if not is_char(self.children[i+1]):
                    char_val = False
            res = '[](const auto &a){return '
            was_break = False
            for i in range(1, len(self.children), 2):
                if self.children[i].token.value(source) in ('E', 'И', 'else', 'иначе'):
                    res += char_or_str(self.children[i+1], char_val)
                    was_break = True
                    break
                res += 'a == ' + char_or_str(self.children[i], char_key) + ' ? ' + char_or_str(self.children[i+1], char_val) + ' : '
                # I L.was_no_break
                #    res ‘’= ‘throw KeyError(a)’
            return res + ('throw KeyError(a)' if not was_break else '') + ';}(' + self.children[0].to_str() + ')'

        if len(self.children) == 1:
            #return '(' + self.symbol.id + self.children[0].to_str() + ')'
            if self.postfix:
                return self.children[0].to_str() + self.symbol.id
            elif self.symbol.id == ':':
                return '::' + self.children[0].to_str()
            elif self.symbol.id == '.':
                c0 = self.children[0].to_str()
                if self.scope.find_in_current_function(c0):
                    return 'this->' + c0
                else:
                    return c0
            elif self.symbol.id == '..':
                return 'range_ei(' + self.children[0].to_str() + ')'
            else:
                return self.symbol.id + self.children[0].to_str()
        elif len(self.children) == 2:
            #return '(' + self.children[0].to_str() + ' ' + self.symbol.id + ' ' + self.children[1].to_str() + ')'

            def char_if_len_1(child):
                if is_char(child):
                    return "u'" + child.token.value(source)[1:-1] + "'_C"
                return child.to_str()

            if self.symbol.id == '.':
                c1 = self.children[1].to_str()
                return char_if_len_1(self.children[0]) + '.' + ('len()' if c1 == 'len' else c1) # char_if_len_1 is needed here because `u"0"_S.code` (have gotten from #(11l)‘‘0’.code’) is illegal [correct: `u'0'_C.code`]
            elif self.symbol.id == '->':
                captured_variables = set()
                def gather_captured_variables(sn):
                    if sn.token.category == Token.Category.NAME:
                        if sn.token.value(source)[0] == '@':
                            captured_variables.add(sn.token.value(source)[1:])
                    else:
                        for child in sn.children:
                            if child != None:
                                gather_captured_variables(child)
                gather_captured_variables(self.children[1])
                return '[' + ', '.join(sorted(captured_variables)) + '](' + ', '.join(map(lambda c: 'const auto &' + c.to_str(), self.children[0].children if self.children[0].symbol.id == '(' else [self.children[0]])) + '){return ' + self.children[1].to_str() + ';}' # )
            elif self.symbol.id == '..':
                return 'range_ee(' + char_if_len_1(self.children[0]) + ', ' + char_if_len_1(self.children[1]) + ')'
            elif self.symbol.id == '.<':
                return 'range_el(' + char_if_len_1(self.children[0]) + ', ' + char_if_len_1(self.children[1]) + ')'
            elif self.symbol.id == '<.':
                return 'range_le(' + char_if_len_1(self.children[0]) + ', ' + char_if_len_1(self.children[1]) + ')'
            elif self.symbol.id == '<.<':
                return 'range_ll(' + char_if_len_1(self.children[0]) + ', ' + char_if_len_1(self.children[1]) + ')'
            elif self.symbol.id in ('C', 'С', 'in'):
                return 'in(' + self.children[0].to_str() + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('I/', 'Ц/'):
                return 'int(' + self.children[0].to_str() + ')/int(' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('==', '!=') and self.children[1].token.category == Token.Category.STRING_LITERAL:
                return self.children[0].to_str() + ' ' + self.symbol.id + ' ' + self.children[1].to_str()[:-2]
            else:
                return self.children[0].to_str() + ' ' + {'&':'&&', '|':'||', '(concat)':'+', '‘’=':'+='}.get(self.symbol.id, self.symbol.id) + ' ' + self.children[1].to_str()
        elif len(self.children) == 3:
            assert(self.symbol.id in ('I', 'Е', 'if', 'если'))
            return self.children[0].to_str() + ' ? ' + self.children[1].to_str() + ' : ' + self.children[2].to_str()

        return ''

symbol_table : Dict[str, SymbolBase] = {}
allowed_keywords_in_expressions : List[str] = []

def symbol(id, bp = 0):
    try:
        s = symbol_table[id]
    except KeyError:
        s = SymbolBase()
        s.id = id
        s.lbp = bp
        symbol_table[id] = s
        if id[0].isalpha() and not id in ('I/', 'Ц/', 'I/=', 'Ц/='): # this is keyword-in-expression
            assert(id.isalpha())
            allowed_keywords_in_expressions.append(id)
    else:
        s.lbp = max(bp, s.lbp)
    return s

class ASTNode:
    parent : 'ASTNode'

    def walk_expressions(self, f):
        pass
    def walk_children(self, f):
        pass

class ASTNodeWithChildren(ASTNode):
    # children : List['ASTNode'] = [] # OMFG! This actually means static (common for all objects of type ASTNode) variable, not default value of member variable, that was unexpected to me as it contradicts C++11 behavior
    children : List['ASTNode']
    tokeni : int

    def __init__(self):
        self.children = []
        self.tokeni = tokeni

    def walk_children(self, f):
        for child in self.children:
            f(child)

    def children_to_str(self, indent, r, place_opening_curly_bracket_on_its_own_line = True):
        r = ('' if self.tokeni == 0 else (source[tokens[self.tokeni-2].end:tokens[self.tokeni].start].count("\n")-1) * "\n") + ' ' * (indent*4) + r + ("\n" + ' ' * (indent*4) + "{\n" if place_opening_curly_bracket_on_its_own_line else " {\n") # }
        for c in self.children:
            r += c.to_str(indent+1)
        return r + ' ' * (indent*4) + "}\n"

    def children_to_str_detect_single_stmt(self, indent, r, check_for_if = False):
        if len(self.children) > 1 or len(self.children) == 0 \
                or (check_for_if and type(self.children[0]) == ASTIf): # for correctly handling of dangling-else
            return self.children_to_str(indent, r, False)
        assert(len(self.children) == 1)
        return ' ' * (indent*4) + r + "\n" + self.children[0].to_str(indent+1)

class ASTNodeWithExpression(ASTNode):
    expression : SymbolNode

    def set_expression(self, expression):
        self.expression = expression
        self.expression.ast_parent = self

    def walk_expressions(self, f):
        f(self.expression)

class ASTProgram(ASTNodeWithChildren):
    def to_str(self):
        r = ''
        for c in self.children:
            r += c.to_str(0)
        return r

class ASTExpression(ASTNodeWithExpression):
    def to_str(self, indent):
        return ' ' * (indent*4) + self.expression.to_str() + ";\n"

cpp_type_from_11l = {'A':'auto', 'А':'auto', 'var':'auto', 'перем':'auto', 'Int':'int', 'String':'String', 'Bool':'bool', 'Array':'Array', 'Tuple':'Tuple'}

class ASTVariableDeclaration(ASTNode):
    var : str
    type : str
    type_args : List[str]

    def to_str(self, indent):
        return ' ' * (indent*4) + cpp_type_from_11l[self.type] + ('<' + ', '.join(cpp_type_from_11l[ty] for ty in self.type_args) + '>' if len(self.type_args) else '') + ' ' + self.var + ";\n"

class ASTVariableInitialization(ASTVariableDeclaration, ASTNodeWithExpression):
    def to_str(self, indent):
        return super().to_str(indent)[:-2] + ' = ' + self.expression.to_str() + ";\n"

class ASTFunctionDefinition(ASTNodeWithChildren):
    function_name : str
    function_arguments : List[Tuple[str, SymbolNode]]# = []

    def __init__(self):
        super().__init__()
        self.function_arguments = []

    def to_str(self, indent):
        if type(self.parent) == ASTClassDefinition:
            if self.function_name == '': # this is constructor
                s = self.parent.class_name
            elif self.function_name == '()': # this is `operator()`
                s = 'auto operator()'
            else:
                s = 'auto ' + self.function_name
        else:
            s = 'auto ' + self.function_name
        return self.children_to_str(indent, s + '()' if len(self.function_arguments) == 0 else
            'template <' + ", ".join(map(lambda index_arg: 'typename T' + str(index_arg[0] + 1), enumerate(self.function_arguments))) + '> ' + s
            + '(' + ", ".join(map(lambda index_arg: 'const T' + str(index_arg[0] + 1) + ' &' + index_arg[1][0] + ('' if index_arg[1][1] == None else ' = ' + index_arg[1][1].to_str()), enumerate(self.function_arguments))) + ')')

class ASTIf(ASTNodeWithChildren, ASTNodeWithExpression):
    else_or_elif : ASTNode = None

    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'if (' + self.expression.to_str() + ')', check_for_if = True) + (self.else_or_elif.to_str(indent) if self.else_or_elif != None else '')

class ASTElseIf(ASTNodeWithChildren, ASTNodeWithExpression):
    else_or_elif : ASTNode = None

    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'else if (' + self.expression.to_str() + ')', check_for_if = True) + (self.else_or_elif.to_str(indent) if self.else_or_elif != None else '')

class ASTElse(ASTNodeWithChildren):
    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'else')

class ASTLoop(ASTNodeWithChildren, ASTNodeWithExpression):
    loop_variable : str = None

    def to_str(self, indent):
        if self.loop_variable != None:
            return self.children_to_str_detect_single_stmt(indent, 'for (auto ' + self.loop_variable + ' : ' + self.expression.to_str() + ')')
        else:
            return self.children_to_str_detect_single_stmt(indent, 'while (' + (self.expression.to_str() if self.expression != None else 'true') + ')')

    def walk_expressions(self, f):
        if self.expression != None: f(self.expression)

class ASTReturn(ASTNodeWithExpression):
    def to_str(self, indent):
        return ' ' * (indent*4) + 'return' + (' ' + self.expression.to_str() if self.expression != None else '') + ";\n"

    def walk_expressions(self, f):
        if self.expression != None: f(self.expression)

class ASTClassDefinition(ASTNodeWithChildren):
    base_classes : List[str]
    class_name : str

    def to_str(self, indent):
        r = ('' if self.tokeni == 0 else (source[tokens[self.tokeni-2].end:tokens[self.tokeni].start].count("\n")-1) * "\n") + ' ' * (indent*4) \
          + 'class ' + self.class_name + (' : ' + ', '.join(map(lambda c: 'public ' + c, self.base_classes)) if len(self.base_classes) else '') \
          + "\n" + ' ' * (indent*4) + "{\n" + ' ' * (indent*4) + "public:\n"
        for c in self.children:
            r += c.to_str(indent+1)
        return r + ' ' * (indent*4) + "};\n"

class ASTMain(ASTNodeWithChildren):
    def to_str(self, indent):
        return self.children_to_str(indent, 'int main()')

def next_token():
    global token, tokeni, tokensn
    if token == None and tokeni != -1:
        raise Error('no more tokens', len(source))
    tokeni += 1
    if tokeni == len(tokens):
        token = None
        tokensn = None
    else:
        token = tokens[tokeni]
        tokensn = SymbolNode(token)
        if token.category != Token.Category.KEYWORD or token.value(source) in allowed_keywords_in_expressions:
            key : str
            if token.category in (Token.Category.NUMERIC_LITERAL, Token.Category.STRING_LITERAL):
                key = '(literal)'
            elif token.category == Token.Category.NAME:
                key = '(name)'
            elif token.category == Token.Category.CONSTANT:
                key = '(constant)'
            elif token.category == Token.Category.STRING_CONCATENATOR:
                key = '(concat)'
            elif token.category == Token.Category.SCOPE_BEGIN:
                key = '{' # }
            elif token.category in (Token.Category.STATEMENT_SEPARATOR, Token.Category.SCOPE_END):
                key = ';'
            else:
                key = token.value(source)
            tokensn.symbol = symbol_table[key]

def advance(value):
    if token.value(source) != value:
        raise Error('expected ' + value, token.start)
    next_token()

def peek_token(how_much = 1):
    return tokens[tokeni+how_much] if tokeni+how_much < len(tokens) else Token()

# This implementation is based on [http://svn.effbot.org/public/stuff/sandbox/topdown/tdop-4.py]
def expression(rbp = 0):
    def check_tokensn():
        if tokensn == None:
            raise Error('unexpected end of source', len(source))
        if tokensn.symbol == None:
            raise Error('no symbol corresponding to token `' + token.value(source) + '` (belonging to ' + str(token.category) +') found while parsing expression', token.start)
    check_tokensn()
    t = tokensn
    next_token()
    check_tokensn()
    left = t.symbol.nud(t)
    while rbp < tokensn.symbol.lbp:
        t = tokensn
        next_token()
        left = t.symbol.led(t, left)
        check_tokensn()
    return left

def infix(id, bp):
    def led(self, left):
        self.append_child(left)
        self.append_child(expression(self.symbol.led_bp))
        return self
    symbol(id, bp).set_led_bp(bp, led)

def infix_r(id, bp):
    def led(self, left):
        self.append_child(left)
        self.append_child(expression(self.symbol.led_bp - 1))
        return self
    symbol(id, bp).set_led_bp(bp, led)

def postfix(id, bp):
    def led(self, left):
        self.postfix = True
        self.append_child(left)
        return self
    symbol(id, bp).led = led

def prefix(id, bp):
    def nud(self):
        self.append_child(expression(self.symbol.nud_bp))
        return self
    symbol(id).set_nud_bp(bp, nud)

infix('[+]', 20); infix('->', 20); infix('(concat)', 25) # s -> s[0]‘’@c = s -> (s[0]‘’@c)

infix('|', 30); infix('&', 40)

infix('==', 50); infix('!=', 50); infix('C', 50); infix('С', 50); infix('in', 50)

infix('..', 55); infix('.<', 55); infix('<.', 55); infix('<.<', 55) # ch C ‘0’..‘9’ = ch C (‘0’..‘9’)
#postfix('..', 55)

infix('<', 60); infix('<=', 60)
infix('>', 60); infix('>=', 60)

infix('[|]', 70); infix('(+)', 80); infix('[&]', 90)

infix('<<', 100); infix('>>', 100)

infix('+', 110); infix('-', 110)

infix('*', 120); infix('/', 120); infix('I/', 120); infix('Ц/', 120)
infix('%', 120)

prefix('-', 130); prefix('+', 130); prefix('!', 130); prefix('--', 130); prefix('++', 130)

infix_r('^', 140)

symbol('.', 150); prefix(':', 150); symbol('[', 150); symbol('(', 150); symbol(')'); symbol(']'); postfix('--', 150); postfix('++', 150)
prefix('.', 150)

infix_r('=', 10); infix_r('+=', 10); infix_r('-=', 10); infix_r('*=', 10); infix_r('/=', 10); infix_r('I/=', 10); infix_r('Ц/=', 10); infix_r('%=', 10); infix_r('>>=', 10); infix_r('<<=', 10); infix_r('**=', 10)
infix_r('[+]=', 10); infix_r('[&]=', 10); infix_r('[|]=', 10); infix_r('(+)=', 10); infix_r('‘’=', 10)

symbol('(name)').nud = lambda self: self
symbol('(literal)').nud = lambda self: self
symbol('(constant)').nud = lambda self: self

symbol(';')
symbol(',')

def led(self, left):
    self.append_child(left) # [
    if token.value(source) != ']':
        self.append_child(expression(self.symbol.led_bp))
    return self
symbol('..', 55).set_led_bp(55, led)

def led(self, left):
    if token.category != Token.Category.NAME:
        raise Error('expected an attribute name', token.start)
    self.append_child(left)
    self.append_child(tokensn)
    next_token()
    return self
symbol('.').led = led

def led(self, left):
    self.function_call = True
    self.append_child(left) # (
    if token.value(source) != ')':
        while True:
            self.append_child(expression())
            if token.value(source) != ',':
                break
            advance(',') # (
    advance(')')
    return self
symbol('(').led = led

def nud(self):
    comma = False # ((
    if token.value(source) != ')':
        while True:
            if token.value(source) == ')':
                break
            self.append_child(expression())
            if token.value(source) != ',':
                break
            comma = True
            advance(',')
    advance(')')
    if len(self.children) == 0 or comma:
        self.tuple = True
    return self
symbol('(').nud = nud # )

def led(self, left):
    self.append_child(left)
    if token.value(source)[0].isupper(): # type name must starts with an upper case letter
        self.is_type = True
        while True:
            self.append_child(expression())
            if token.value(source) != ',':
                break
            advance(',')
    else:
        self.append_child(expression()) # [
    advance(']')
    return self
symbol('[').led = led

def nud(self):
    if peek_token().value(source) == '=':
        self.is_dict = True
        while True: # [
            self.append_child(expression())
            if token.value(source) != ',':
                break
            advance(',')
        advance(']')
    else:
        self.is_list = True
        if token.value(source) != ']':
            while True: # [[
                # if token.value(source) == ']':
                #     break
                self.append_child(expression())
                if token.value(source) != ',':
                    break
                advance(',')
        advance(']')
    return self
symbol('[').nud = nud # ]

def advance_scope_begin():
    if token.category != Token.Category.SCOPE_BEGIN:
        raise Error('expected a new scope (indented block or opening curly bracket)', token.start)
    next_token()

def nud(self):
    self.append_child(expression())
    advance_scope_begin()
    while token.category != Token.Category.SCOPE_END:
        if token.value(source) in ('E', 'И', 'else', 'иначе'):
            self.append_child(tokensn)
            next_token()
            if token.category == Token.Category.SCOPE_BEGIN:
                next_token()
                self.append_child(expression())
                if token.category != Token.Category.SCOPE_END:
                    raise Error('expected end of scope (dedented block or closing curly bracket)', token.start)
                next_token()
            else:
                self.append_child(expression())
        else:
            self.append_child(expression())
            advance_scope_begin()
            self.append_child(expression())
            if token.category != Token.Category.SCOPE_END:
                raise Error('expected end of scope (dedented block or closing curly bracket)', token.start)
            next_token()
            if token.category == Token.Category.STATEMENT_SEPARATOR:
                next_token()
    next_token()
    return self
symbol('S').nud = nud
symbol('В').nud = nud
symbol('switch').nud = nud
symbol('выбрать').nud = nud

def nud(self):
    self.append_child(expression())
    advance_scope_begin()
    self.append_child(expression())
    if token.category != Token.Category.SCOPE_END:
        raise Error('expected end of scope (dedented block or closing curly bracket)', token.start)
    next_token()
    if not token.value(source) in ('E', 'И', 'else', 'иначе'):
        raise Error('expected else block', token.start)
    next_token()
    self.append_child(expression())
    return self
symbol('I').nud = nud
symbol('Е').nud = nud
symbol('if').nud = nud
symbol('если').nud = nud

symbol('{') # }

def parse_internal(this_node):
    global token

    def new_scope(node, func_args = None):
        advance_scope_begin()
        global scope
        prev_scope = scope
        scope = Scope(func_args)
        scope.parent = prev_scope
        tokensn.scope = scope # можно избавиться от этой строки, если не делать вызов next_token() в advance_scope_begin()
        parse_internal(node)
        scope = prev_scope
        if token != None:
            tokensn.scope = scope

    def expected_name(what_name):
        next_token()
        if token.category != Token.Category.NAME:
            raise Error('expected ' + what_name, token.start)
        token_value = token.value(source)
        next_token()
        return token_value

    while token != None:
        if token.value(source) == ':' and peek_token().value(source) in ('start', 'старт'):
            node = ASTMain()
            next_token()
            next_token()
            advance(':')
            assert(token.category == Token.Category.STATEMENT_SEPARATOR)
            next_token()
            parse_internal(node)
        elif token.category == Token.Category.KEYWORD:
            if token.value(source) in ('F', 'Ф', 'fn', 'фн'):
                node = ASTFunctionDefinition()
                next_token()
                if token.category == Token.Category.NAME:
                    node.function_name = token.value(source)
                    next_token()
                elif token.value(source) == '(': # this is constructor [`F ()...` or `F (...)...`] or operator() [`F ()(...)...`]
                    if peek_token().value(source) == ')' and peek_token(2).value(source) == '(': # ) # this is operator()
                        next_token()
                        next_token()
                        node.function_name = '()'
                    else:
                        node.function_name = ''
                elif token.category == Token.Category.OPERATOR:
                    node.function_name = token.value(source)
                    next_token()
                else:
                    raise Error('incorrect function name', token.start)

                if token.value(source) != '(': # )
                    raise Error('expected `(` after function name', token.start) # )(

                next_token()
                while token.value(source) != ')':
                    if token.category != Token.Category.NAME:
                        raise Error('expected function\'s argument name', token.start)
                    func_arg_name = token.value(source)
                    next_token()
                    if token.value(source) == '=':
                        next_token()
                        default = expression()
                    else:
                        default = None
                    node.function_arguments.append((func_arg_name, default)) # ((
                    if token.value(source) not in ',)':
                        raise Error('expected `,` or `)` in function\'s arguments list', token.start)
                    if token.value(source) == ',':
                        next_token()

                next_token()
                new_scope(node, map(lambda arg: arg[0], node.function_arguments))

            elif token.value(source) in ('T', 'Т', 'type', 'тип'):
                node = ASTClassDefinition()
                node.class_name = expected_name('class name')
                node.base_classes = []

                if token.value(source) == '(':
                    while True:
                        node.base_classes.append(expected_name('base class name'))
                        if token.value(source) != ',':
                            break
                    if token.value(source) != ')': # (
                        raise Error('expected `)`', token.start)
                    next_token()

                new_scope(node)

            elif token.value(source) in ('I', 'Е', 'if', 'если'):
                node = ASTIf()
                next_token()
                node.set_expression(expression())
                new_scope(node)

                n = node
                while token != None and token.value(source) in ('E', 'И', 'else', 'иначе'):
                    if peek_token().value(source) in ('I', 'Е', 'if', 'если'):
                        n.else_or_elif = ASTElseIf()
                        n = n.else_or_elif
                        next_token()
                        next_token()
                        n.set_expression(expression())
                        new_scope(n)
                    if token != None and token.value(source) in ('E', 'И', 'else', 'иначе') and not peek_token().value(source) in ('I', 'Е', 'if', 'если'):
                        n.else_or_elif = ASTElse()
                        next_token()
                        new_scope(n.else_or_elif)
                        break

            elif token.value(source) in ('L', 'Ц', 'loop', 'цикл'):
                node = ASTLoop()
                next_token()
                if token.category == Token.Category.SCOPE_BEGIN:
                    node.expression = None
                else:
                    if token.value(source) == '(':
                        node.loop_variable = expected_name('loop variable')
                        advance(')')
                    node.set_expression(expression())
                new_scope(node)

            elif token.value(source) in ('R', 'Р', 'return', 'вернуть'):
                node = ASTReturn()
                next_token()
                if token.category in (Token.Category.SCOPE_END, Token.Category.STATEMENT_SEPARATOR):
                    node.expression = None
                else:
                    node.set_expression(expression())
                if token != None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            else:
                raise Error('unrecognized statement started with keyword', token.start)

        elif token.category == Token.Category.SCOPE_END:
            next_token()
            if token.category == Token.Category.STATEMENT_SEPARATOR: # Token.Category.EOF
                next_token()
                assert(token == None)
            return

        else:
            node_expression = expression()
            if token.category == Token.Category.NAME:
                var_name = token.value(source)
                next_token()
                if token.value(source) == '=':
                    next_token()
                    node = ASTVariableInitialization()
                    node.set_expression(expression())
                else:
                    node = ASTVariableDeclaration()
                node.var = var_name
                node.type = node_expression.token.value(source)
                node.type_args = []
                if node.type == '[': # ]
                    node.type = node_expression.children[0].token.value(source)
                    for i in range(1, len(node_expression.children)):
                        node.type_args.append(node_expression.children[i].to_str())
                assert(node.type[0].isupper() or node.type in ('var', 'перем')) # type name must starts with an upper case letter
            else:
                node = ASTExpression()
                node.set_expression(node_expression)
            if not (token == None or token.category in (Token.Category.STATEMENT_SEPARATOR, Token.Category.SCOPE_END)):
                raise Error('expected end of statement', token.start)
            if token != None and token.category == Token.Category.STATEMENT_SEPARATOR:
                next_token()

        node.parent = this_node
        this_node.children.append(node)

    return

def parse(tokens_, source_, suppress_error_please_wrap_in_copy = False): # option suppress_error_please_wrap_in_copy is needed to simplify conversion of large Python source into C++
    global tokens, source, tokeni, token, scope
    tokens = tokens_ + [Token(len(source_), len(source_), Token.Category.STATEMENT_SEPARATOR)]
    source = source_
    tokeni = -1
    token = None
    scope = Scope(None)
    next_token()
    p = ASTProgram()
    if len(tokens_) == 0: return p
    parse_internal(p)
    return p
