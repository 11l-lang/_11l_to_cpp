from tokenizer import Token
from typing import List, Tuple, Dict, Callable

class Error(Exception):
    def __init__(self, message, pos):
        self.message = message
        self.pos = pos

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
    postfix : bool = False

    def __init__(self, token):
        self.token = token
        self.children = []

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
            if self.is_list:
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
            return self.token.value(source)

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
            return '"' + repr(self.token.value(source)[1:-1])[1:-1].replace('"', R'\"').replace(R"\'", "'") + '"'

        if self.token.category == Token.Category.CONSTANT:
            return {'N': 'nullptr', 'Н': 'nullptr', '0B': 'false', '0В': 'false', '1B': 'true', '1В': 'true'}[self.token.value(source)]

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
                res = 'Tuple('
                for i in range(len(self.children)):
                    res += self.children[i].to_str()
                    if i < len(self.children)-1:
                        res += ', '
                if len(self.children) == 1:
                    res += ','
                return res + ')'
            else:
                assert(len(self.children) == 1)
                return '(' + self.children[0].to_str() + ')'

        elif self.symbol.id in ('S', 'В', 'switch', 'выбрать'):
            res = '[](const auto &a){return '
            for i in range(1, len(self.children), 2):
                res += 'a == ' + self.children[i].to_str() + ' ? ' + self.children[i+1].to_str() + ' : '
            return res + 'throw KeyError(a);}(' + self.children[0].to_str() + ')'

        if len(self.children) == 1:
            #return '(' + self.symbol.id + self.children[0].to_str() + ')'
            if self.postfix:
                return self.children[0].to_str() + self.symbol.id
            elif self.symbol.id == ':':
                return '::' + self.children[0].to_str()
            else:
                return self.symbol.id + self.children[0].to_str()
        elif len(self.children) == 2:
            #return '(' + self.children[0].to_str() + ' ' + self.symbol.id + ' ' + self.children[1].to_str() + ')'
            return self.children[0].to_str() + ' ' + {'&':'&&', '|':'||'}.get(self.symbol.id, self.symbol.id) + ' ' + self.children[1].to_str()
        elif len(self.children) == 3:
            assert(self.symbol.id == 'I')

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

    def children_to_str(self, indent, r):
        r = ('' if self.tokeni == 0 else (source[tokens[self.tokeni-2].end:tokens[self.tokeni].start].count("\n")-1) * "\n") + ' ' * (indent*4) + r + "\n" + ' ' * (indent*4) + "{\n"
        for c in self.children:
            r += c.to_str(indent+1)
        return r + ' ' * (indent*4) + "}\n"

    def children_to_str_detect_single_stmt(self, indent, r):
        if len(self.children) > 1:
            return self.children_to_str(indent, r)
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
        return ' ' * (indent*4) + self.expression.to_str() + "\n"

cpp_type_from_11l = {'Int':'int', 'String':'String', 'Bool':'bool', 'Array':'Array', 'Tuple':'Tuple'}

class ASTVariableDeclaration(ASTNode):
    var : str
    type : str
    type_args : List[str]

    def to_str(self, indent):
        return ' ' * (indent*3) + cpp_type_from_11l[self.type] + ('<' + ', '.join(cpp_type_from_11l[ty] for ty in self.type_args) + '>' if len(self.type_args) else '') + ' ' + self.var + ";\n"

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
        return self.children_to_str(indent, 'auto ' + self.function_name + '()' if len(self.function_arguments) == 0 else
            'template <' + ", ".join(map(lambda index_arg: 'typename T' + str(index_arg[0] + 1), enumerate(self.function_arguments))) + '> auto ' + self.function_name
            + '(' + ", ".join(map(lambda index_arg: 'const T' + str(index_arg[0] + 1) + ' &' + index_arg[1][0] + ('' if index_arg[1][1] == None else ' = ' + index_arg[1][1].to_str()), enumerate(self.function_arguments))) + ')')

class ASTIf(ASTNodeWithChildren, ASTNodeWithExpression):
    else_or_elif : ASTNode = None

    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'if (' + self.expression.to_str() + ')') + (self.else_or_elif.to_str(indent) if self.else_or_elif != None else '')

class ASTElseIf(ASTNodeWithChildren, ASTNodeWithExpression):
    else_or_elif : ASTNode = None

    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'else if (' + self.expression.to_str() + ')') + (self.else_or_elif.to_str(indent) if self.else_or_elif != None else '')

class ASTElse(ASTNodeWithChildren):
    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'else')

class ASTReturn(ASTNodeWithExpression):
    def to_str(self, indent):
        return ' ' * (indent*4) + 'return' + (' ' + self.expression.to_str() if self.expression != None else '') + ";\n"

    def walk_expressions(self, f):
        if self.expression != None: f(self.expression)

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

infix('|', 30); infix('&', 40);

infix('==', 50); infix('!=', 50)

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

infix_r('=', 10); infix_r('+=', 10); infix_r('-=', 10); infix_r('*=', 10); infix_r('/=', 10); infix_r('I/=', 10); infix_r('Ц/=', 10); infix_r('%=', 10); infix_r('>>=', 10); infix_r('<<=', 10); infix_r('**=', 10)

symbol('(name)').nud = lambda self: self
symbol('(literal)').nud = lambda self: self
symbol('(constant)').nud = lambda self: self

symbol(';')
symbol(',')

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
    self.append_child(expression()) # [
    advance(']')
    return self
symbol('[').led = led

def nud(self):
    self.is_list = True
    if token.value(source) != ']':
        while True: # [[
            if token.value(source) == ']':
                break
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

symbol('{') # }

def parse_internal(this_node):
    global token

    def new_scope(node, func_args = None):
        advance_scope_begin()
        parse_internal(node)

    while token != None:
        if token.category == Token.Category.KEYWORD:
            if token.value(source) in ('F', 'Ф', 'fn', 'фн'):
                node = ASTFunctionDefinition()
                next_token()
                if token.category == Token.Category.NAME:
                    node.function_name = token.value(source)
                    next_token()
                elif token.symbol.id == '(': # this is constructor [`F ()...` or `F (...)...`] or operator() [`F ()(...)...`]
                    if peek_token().symbol.id == ')' and peek_token(2).symbol.id == '(': # ) # this is operator()
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
                assert(node.type[0].isupper()) # type name must begins with upper case letter
                assert(node_expression.token.category == Token.Category.NAME) # [-add support of template types-]
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

def parse(tokens_, source_):
    global tokens, source, tokeni, token, scope
    tokens = tokens_ + [Token(len(source_), len(source_), Token.Category.STATEMENT_SEPARATOR)]
    source = source_
    tokeni = -1
    token = None
    next_token()
    p = ASTProgram()
    if len(tokens_) == 0: return p
    parse_internal(p)
    return p
