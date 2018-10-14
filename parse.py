from tokenizer import Token
import tokenizer
from typing import List, Tuple, Dict, Callable
import os

class Error(Exception):
    def __init__(self, message, token):
        self.message = message
        self.pos = token.start
        self.end = token.end

class Scope:
    parent : 'Scope'
    class Id:
        type : str
        ast_nodes : List['ASTNodeWithChildren']

        def __init__(self, type, ast_node = None):
            self.type = type
            self.ast_nodes = []
            if ast_node != None:
                self.ast_nodes.append(ast_node)
    ids : Dict[str, Id]
    is_function : bool

    def __init__(self, func_args):
        self.parent = None
        if func_args != None:
            self.is_function = True
            self.ids = dict(map(lambda x: (x[0], Scope.Id(x[1])), func_args))
        else:
            self.is_function = False
            self.ids = {}

    def find_in_current_function(self, name):
        s = self
        while True:
            if name in s.ids:
                return True
            if s.is_function:
                return False
            s = s.parent
            if s == None:
                return False

    def find(self, name):
        s = self
        while True:
            id = s.ids.get(name)
            if id != None:
                return id
            s = s.parent
            if s == None:
                return None

    def add_function(self, name, ast_node):
        if name in self.ids:                                                   # A &id = .ids.set_if_not_present(name, Id(N)) // note that this is an error: `A id = .ids.set_if_not_present(...)`, but you can do this: `A id = copy(.ids.set_if_not_present(...))`
            assert(type(self.ids[name].ast_nodes[0]) == ASTFunctionDefinition) # assert(id.ast_nodes.empty | T(id.ast_nodes[0]) == ASTFunctionDefinition)
            self.ids[name].ast_nodes.append(ast_node)                          # id.ast_nodes [+]= ast_node
        else:
            self.ids[name] = Scope.Id(None, ast_node)

    def add_name(self, name, ast_node):
        if name in self.ids:                                                                                    # I !.ids.set(name, Id(N, ast_node))
            raise Error('redefinition of already defined identifier is not allowed', tokens[ast_node.tokeni+1]) #    X Error(‘redefinition ...’, ...)
        self.ids[name] = Scope.Id(None, ast_node)

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
        def nud(s): raise Error('unknown unary operator', s.token)
        self.nud = nud
        def led(s, l): raise Error('unknown binary operator', s.token)
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
    token_str_override : str

    def __init__(self, token, token_str_override = None):
        self.token = token
        self.children = []
        self.scope = scope
        self.token_str_override = token_str_override

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
            return (self.children[-1] or self.children[-2]).rightmost() + 1

        return self.children[-1].rightmost()

    def token_str(self):
        return self.token.value(source) if not self.token_str_override else self.token_str_override

    def to_str(self):
        if self.token.category == Token.Category.NAME:
            return self.token_str().lstrip('@')

        if self.token.category == Token.Category.NUMERIC_LITERAL:
            n = self.token_str()
            if n[-1] in 'oо':
                return '0' + n[:-1]
            if n[-1] in 'bд':
                return '0b' + n[:-1]
            if n[4:5] == "'" or n[-3:-2] == "'":
                nn = ''
                for c in n:
                    nn += {'А':'A','Б':'B','С':'C','Д':'D','Е':'E','Ф':'F'}.get(c, c)
                return '0x' + nn
            return n

        if self.token.category == Token.Category.STRING_LITERAL:
            s = self.token_str()
            if s[0] == '"':
                return 'u' + s + '_S'

            eat_left = 0
            while s[eat_left] == "'":
                eat_left += 1
            eat_right = 0
            while s[-1-eat_right] == "'":
                eat_right += 1
            s = s[1+eat_left*2:-1-eat_right*2]

            if '\\' in s or "\n" in s:
                delimiter = '' # (
                while ')' + delimiter + '"' in s:
                    delimiter += "'"
                return 'uR"' + delimiter + '(' + s + ')' + delimiter + '"_S'

            return 'u"' + repr(s)[1:-1].replace('"', R'\"').replace(R"\'", "'") + '"_S'

        if self.token.category == Token.Category.CONSTANT:
            return {'N': 'nullptr', 'Н': 'nullptr', '0B': 'false', '0В': 'false', '1B': 'true', '1В': 'true'}[self.token_str()]

        def is_char(child):
            ts = child.token_str()
            return child.token.category == Token.Category.STRING_LITERAL and (len(ts) == 3 or (ts[:2] == '"\\' and len(ts) == 4))

        def char_or_str(child, is_char):
            if is_char:
                return "u'" + child.token_str()[1:-1].replace("'", R"\'") + "'_C"
            return child.to_str()

        if self.symbol.id == '(': # )
            if self.function_call:
                func_name = self.children[0].to_str()
                f_node = None
                if self.children[0].symbol.id == '.':
                    if len(self.children[0].children) == 1:
                        s = self.scope
                        while True:
                            if s.is_function:
                                for id in s.parent.ids.values():
                                    if type(id.ast_nodes[0].parent) == ASTTypeDefinition:
                                        fid = s.parent.ids.get(self.children[0].children[0].to_str())
                                        if fid == None:
                                            raise Error('call of undefined method `' + func_name + '`', self.children[0].children[0].token)
                                        if len(fid.ast_nodes) > 1:
                                            raise Error('methods\' overloading is not supported for now', self.children[0].children[0].token)
                                        f_node = fid.ast_nodes[0]
                                    break
                                break
                            s = s.parent
                            assert(s)
                elif func_name == 'Int':
                    func_name = 'parse_int'
                else:
                    if self.children[0].symbol.id == ':':
                        fid = modules[self.children[0].children[0].token_str()].scope.find(self.children[0].children[1].token_str())
                    else:
                        fid = self.scope.find(func_name)
                    if fid == None:
                        raise Error('call of undefined function `' + func_name + '`', self.children[0].token)
                    if len(fid.ast_nodes) > 1:
                        raise Error('functions\' overloading is not supported for now', self.children[0].token)
                    f_node = fid.ast_nodes[0]
                    assert(type(f_node) in (ASTFunctionDefinition, ASTTypeDefinition) or (type(f_node) in (ASTVariableInitialization, ASTVariableDeclaration) and f_node.function_pointer))
                    if type(f_node) == ASTTypeDefinition:
                        if len(f_node.constructors) == 0:
                            f_node = ASTFunctionDefinition()
                        else:
                            if len(f_node.constructors) > 1:
                                raise Error('constructors\' overloading is not supported for now (see type `' + f_node.type_name + '`)', self.children[0].token)
                            f_node = f_node.constructors[0]
                last_function_arg = 0
                res = func_name + '('
                for i in range(1, len(self.children), 2):
                    if self.children[i] == None:
                        if f_node != None and type(f_node) == ASTFunctionDefinition and f_node.function_arguments[last_function_arg][2].endswith('?'):
                            res += '&'
                        res += self.children[i+1].to_str()
                        last_function_arg += 1
                    else:
                        argument_name = self.children[i].to_str()[:-1]
                        while True:
                            if f_node == None:
                                raise Error('function `' + func_name + '` is not found', Token(self.children[0].leftmost(), self.children[0].rightmost(), Token.Category.NAME))
                            if last_function_arg == len(f_node.function_arguments):
                                raise Error('argument `' + argument_name + '` is not found in function `' + func_name + '`', self.children[i].token)
                            if f_node.function_arguments[last_function_arg][0] == argument_name:
                                last_function_arg += 1
                                break
                            if f_node.function_arguments[last_function_arg][1] == '':
                                raise Error('argument `' + f_node.function_arguments[last_function_arg][0] + '` of function `' + func_name + '` has no default value, please specify its value here', self.children[i].token)
                            res += f_node.function_arguments[last_function_arg][1] + ', '
                            last_function_arg += 1
                        res += self.children[i+1].to_str()
                    if i < len(self.children)-2:
                        res += ', '

                if f_node != None:
                    if type(f_node) == ASTFunctionDefinition:
                        while last_function_arg < len(f_node.function_arguments):
                            if f_node.function_arguments[last_function_arg][1] == '':
                                t = self.children[len(self.children)-1].token
                                raise Error('missing required argument `'+ f_node.function_arguments[last_function_arg][0] + '`', Token(t.end, t.end, Token.Category.DELIMITER))
                            last_function_arg += 1
                    else:
                        if last_function_arg != len(f_node.type_args):
                            raise Error('wrong number of arguments passed to function pointer', Token(self.children[0].token.end, self.children[0].token.end, Token.Category.DELIMITER))

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
                c1 = self.children[1].to_str()
                if c1.startswith('(len)'):
                    return self.children[0].to_str() + '.at_plus_len(' + c1[len('(len)'):] + ')'
                return self.children[0].to_str() + '[' + c1 + ']'

        elif self.symbol.id in ('S', 'В', 'switch', 'выбрать'):
            char_val = True
            for i in range(1, len(self.children), 2):
                if not is_char(self.children[i+1]):
                    char_val = False
            res = '[&](const auto &a){return ' # `[&]` is for `cc = {'а':'A','б':'B','с':'C','д':'D','е':'E','ф':'F'}.get(c.lower(), c)` -> `[&](const auto &a){return a == u'а'_C ? u"A"_S : ... : c;}(c.lower())`
            was_break = False
            for i in range(1, len(self.children), 2):
                if self.children[i].token.value(source) in ('E', 'И', 'else', 'иначе'):
                    res += char_or_str(self.children[i+1], char_val)
                    was_break = True
                    break
                res += 'a == ' + char_or_str(self.children[i], is_char(self.children[i]))[:-2] + ' ? ' + char_or_str(self.children[i+1], char_val) + ' : '
                # L.was_no_break
                #    res ‘’= ‘throw KeyError(a)’
            return res + ('throw KeyError(a)' if not was_break else '') + ';}(' + self.children[0].to_str() + ')'

        if len(self.children) == 1:
            #return '(' + self.symbol.id + self.children[0].to_str() + ')'
            if self.postfix:
                return self.children[0].to_str() + self.symbol.id
            elif self.symbol.id == ':':
                c0 = self.children[0].to_str()
                if c0 in ('stdin', 'stdout', 'stderr'):
                    return '_' + c0
                return '::' + c0
            elif self.symbol.id == '.':
                c0 = self.children[0].to_str()
                if self.scope.find_in_current_function(c0):
                    return 'this->' + c0
                else:
                    return c0
            elif self.symbol.id == '..':
                c0 = self.children[0].to_str()
                if c0.startswith('(len)'):
                    return 'range_elen_i(' + c0[len('(len)'):] + ')'
                else:
                    return 'range_ei(' + c0 + ')'
            else:
                return {'(-)':'~'}.get(self.symbol.id, self.symbol.id) + self.children[0].to_str()
        elif len(self.children) == 2:
            #return '(' + self.children[0].to_str() + ' ' + self.symbol.id + ' ' + self.children[1].to_str() + ')'

            def char_if_len_1(child):
                if is_char(child):
                    return "u'" + child.token.value(source)[1:-1].replace("'", R"\'") + "'_C"
                return child.to_str()

            if self.symbol.id == '.':
                cts0 = self.children[0].token_str()
                c1 = self.children[1].to_str()
                if cts0 == '@':
                    if self.scope.find_in_current_function(c1):
                        return 'this->' + c1
                    else:
                        return c1
                id = self.scope.find(cts0.lstrip('@'))
                if id != None and id.type != None and id.type.endswith('?'):
                    return cts0.lstrip('@') + '->' + c1
                return char_if_len_1(self.children[0]) + '.' + c1 + ('()' if c1 in ('len', 'last', 'empty') else '') # char_if_len_1 is needed here because `u"0"_S.code` (have gotten from #(11l)‘‘0’.code’) is illegal [correct: `u'0'_C.code`]
            elif self.symbol.id == ':':
                return self.children[0].to_str() + '::' + self.children[1].to_str()
            elif self.symbol.id == '->':
                captured_variables = set()
                def gather_captured_variables(sn):
                    if sn.token.category == Token.Category.NAME:
                        if sn.token.value(source)[0] == '@':
                            by_ref = True # sn.parent.children[0] is sn and ((sn.parent.symbol.id[-1] == '=' and sn.parent.symbol.id not in ('==', '!='))
                                          #                               or (sn.parent.symbol.id == '.' and sn.parent.children[1].token_str() == 'append'))
                            captured_variables.add(('&' if by_ref else '') + sn.token.value(source)[1:])
                    else:
                        for child in sn.children:
                            if child != None and child.symbol.id != '->':
                                gather_captured_variables(child)
                gather_captured_variables(self.children[1])
                return '[' + ', '.join(sorted(captured_variables)) + '](' + ', '.join(map(lambda c: 'const auto &' + c.to_str(), self.children[0].children if self.children[0].symbol.id == '(' else [self.children[0]])) + '){return ' + self.children[1].to_str() + ';}' # )
            elif self.symbol.id in ('..', '.<', '<.', '<.<'):
                s = {'..':'ee', '.<':'el', '<.':'le', '<.<':'ll'}[self.symbol.id]
                c0 = char_if_len_1(self.children[0])
                c1 = char_if_len_1(self.children[1])
                b = s[0]
                if c0.startswith('(len)'):
                    b += 'len'
                    c0 = c0[len('(len)'):]
                e = s[1]
                if c1.startswith('(len)'):
                    e += 'len'
                    c1 = c1[len('(len)'):]
                return 'range_' + b + '_'*(len(b) > 1 or len(e) > 1) + e + '(' + c0 + ', ' + c1 + ')'
            elif self.symbol.id in ('C', 'С', 'in'):
                return 'in(' + char_if_len_1(self.children[0]) + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('!C', '!С', '!in'):
                return '!in(' + char_if_len_1(self.children[0]) + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('I/', 'Ц/'):
                return 'int(' + self.children[0].to_str() + ')/int(' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('==', '!=') and self.children[1].token.category == Token.Category.STRING_LITERAL:
                return self.children[0].to_str() + ' ' + self.symbol.id + ' ' + char_if_len_1(self.children[1])[:-2]
            elif self.symbol.id == '=' and self.children[0].symbol.id == '[': # ] # replace `a[k] = v` with `a.set(k, v)`
                c01 = self.children[0].children[1].to_str()
                if c01.startswith('(len)'):
                    return self.children[0].children[0].to_str() + '.set_plus_len(' + c01[len('(len)'):] + ', ' + char_if_len_1(self.children[1]) + ')'
                else:
                    return self.children[0].children[0].to_str() + '.set(' + c01 + ', ' + char_if_len_1(self.children[1]) + ')'
            elif self.symbol.id == '[+]=': # replace `a [+]= v` with `a.append(v)`
                return self.children[0].to_str() + '.append(' + self.children[1].to_str() + ')'
            else:
                return self.children[0].to_str() + ' ' + {'&':'&&', '|':'||', '(concat)':'+', '‘’=':'+=', '(+)':'^'}.get(self.symbol.id, self.symbol.id) + ' ' + self.children[1].to_str()
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
        if id[0].isalpha() and not id in ('I/', 'Ц/', 'I/=', 'Ц/=', 'C', 'С', 'in'): # this is keyword-in-expression
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

    def children_to_str(self, indent, t, place_opening_curly_bracket_on_its_own_line = True, add_at_beginning = ''):
        r = ''
        if self.tokeni > 0:
            ti = self.tokeni - 1
            while ti > 0 and tokens[ti].category in (Token.Category.SCOPE_END, Token.Category.STATEMENT_SEPARATOR):
                ti -= 1
            r = (source[tokens[ti].end:tokens[self.tokeni].start].count("\n")-1) * "\n"
        r += ' ' * (indent*4) + t + ("\n" + ' ' * (indent*4) + "{\n" if place_opening_curly_bracket_on_its_own_line else " {\n") # }
        r += add_at_beginning
        for c in self.children:
            r += c.to_str(indent+1)
        return r + ' ' * (indent*4) + "}\n"

    def children_to_str_detect_single_stmt(self, indent, r, check_for_if = False):
        if len(self.children) != 1 \
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
    beginning_extra = ''

    def to_str(self):
        r = self.beginning_extra
        for c in self.children:
            r += c.to_str(0)
        return r

class ASTExpression(ASTNodeWithExpression):
    def to_str(self, indent):
        return ' ' * (indent*4) + self.expression.to_str() + ";\n"

cpp_type_from_11l = {'A':'auto', 'А':'auto', 'var':'auto', 'перем':'auto',
                     'Int':'int', 'String':'String', 'Bool':'bool',
                     'N':'void', 'Н':'void', 'null':'void', 'нуль':'void',
                     'Array':'Array', 'Tuple':'Tuple', 'Dict':'Dict',
                     'Array[String]':'Array<String>', 'Array[Array[String]]':'Array<Array<String>>'}

class ASTVariableDeclaration(ASTNode):
    var : str
    type : str
    type_args : List[str]
    function_pointer : bool = False

    def to_str(self, indent):
        if self.function_pointer:
            return ' ' * (indent*4) + 'std::function<' + cpp_type_from_11l[self.type] + '(' + ', '.join(cpp_type_from_11l[ty] for ty in self.type_args) + ')> ' + self.var + ";\n"
        return ' ' * (indent*4) + cpp_type_from_11l[self.type] + ('<' + ', '.join(cpp_type_from_11l[ty] for ty in self.type_args) + '>' if len(self.type_args) else '') + ' ' + self.var + ";\n"

class ASTVariableInitialization(ASTVariableDeclaration, ASTNodeWithExpression):
    def to_str(self, indent):
        return super().to_str(indent)[:-2] + ' = ' + self.expression.to_str() + ";\n"

class ASTFunctionDefinition(ASTNodeWithChildren):
    function_name : str
    function_return_type : str = ''
    function_arguments : List[Tuple[str, str, str, str]]# = [] # (arg_name, default_value, type_, qualifiers)
    first_named_only_argument = None
    last_non_default_argument : int

    def __init__(self, function_arguments = None):
        super().__init__()
        self.function_arguments = function_arguments or []

    def to_str(self, indent):
        if type(self.parent) == ASTTypeDefinition:
            if self.function_name == '': # this is constructor
                s = self.parent.type_name
            elif self.function_name == '()': # this is `operator()`
                s = ('auto' if self.function_return_type == '' else cpp_type_from_11l[self.function_return_type]) + ' operator()'
            else:
                s = ('auto' if self.function_return_type == '' else cpp_type_from_11l[self.function_return_type]) + ' ' + self.function_name

        elif type(self.parent) != ASTProgram: # local functions [i.e. functions inside functions] are represented as C++ lambdas
            captured_variables = set()
            def gather_captured_variables(node):
                def f(sn : SymbolNode):
                    if sn.token.category == Token.Category.NAME:
                        if sn.token.value(source)[0] == '@':
                            by_ref = True # sn.parent and sn.parent.children[0] is sn and sn.parent.symbol.id[-1] == '=' and sn.parent.symbol.id not in ('==', '!=')
                            t = sn.token.value(source)[1:]
                            captured_variables.add('this' if t == '' else '&'*by_ref + t)
                    else:
                        for child in sn.children:
                            if child != None:
                                f(child)

                node.walk_expressions(f)
                node.walk_children(gather_captured_variables)
            gather_captured_variables(self)

            arguments = []
            for arg in self.function_arguments:
                if arg[2] == '': # if there is no type specified
                    arguments.append(('auto ' if '=' in arg[3] else 'const auto &') + arg[0] if arg[1] == '' else
                                          ('' if '=' in arg[3] else 'const ') + 'decltype(' + arg[1] + ') ' + arg[0] + ' = ' + arg[1])
                else:
                    arguments.append(('' if '=' in arg[3] else 'const ') + cpp_type_from_11l[arg[2]] + ' ' + ('&'*(arg[2] not in ('Int',))) + arg[0] + ('' if arg[1] == '' else ' = ' + arg[1]))
            return self.children_to_str(indent, ('auto' if self.function_return_type == '' else cpp_type_from_11l[self.function_return_type]) + ' ' + self.function_name
                + ' = [' + ', '.join(sorted(filter(lambda v: not '&'+v in captured_variables, captured_variables))) + '](' \
                + ', '.join(arguments) + ')')[:-1] + ";\n"

        else:
            s = ('auto' if self.function_return_type == '' else cpp_type_from_11l[self.function_return_type]) + ' ' + self.function_name

        if len(self.function_arguments) == 0:
            return self.children_to_str(indent, s + '()')

        templates = []
        arguments = []
        for index, arg in enumerate(self.function_arguments):
            if arg[2] == '': # if there is no type specified
                templates.append('typename T' + str(index + 1) + ('' if arg[1] == '' or index < self.last_non_default_argument else ' = decltype(' + arg[1] + ')'))
                arguments.append(('T' + str(index + 1) + ' ' if '=' in arg[3] else 'const T' + str(index + 1) + ' &')
                    + arg[0] + ('' if arg[1] == '' or index < self.last_non_default_argument else ' = ' + arg[1]))
            else:
                arguments.append(arg[2].rstrip('?') + '* '
                    + ('' if '=' in arg[3] else 'const ')
                    + arg[0] + ('' if arg[1] == '' or index < self.last_non_default_argument else ' = ' + arg[1]))
        return self.children_to_str(indent, 'template <' + ', '.join(templates) + '> ' + s + '(' + ', '.join(arguments) + ')')

class ASTIf(ASTNodeWithChildren, ASTNodeWithExpression):
    else_or_elif : ASTNode = None

    def walk_children(self, f):
        super().walk_children(f)
        if self.else_or_elif != None:
            self.else_or_elif.walk_children(f)

    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'if (' + self.expression.to_str() + ')', check_for_if = True) + (self.else_or_elif.to_str(indent) if self.else_or_elif != None else '')

class ASTElseIf(ASTNodeWithChildren, ASTNodeWithExpression):
    else_or_elif : ASTNode = None

    def walk_children(self, f):
        super().walk_children(f)
        if self.else_or_elif != None:
            self.else_or_elif.walk_children(f)

    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'else if (' + self.expression.to_str() + ')', check_for_if = True) + (self.else_or_elif.to_str(indent) if self.else_or_elif != None else '')

class ASTElse(ASTNodeWithChildren):
    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'else')

class ASTSwitch(ASTNodeWithExpression):
    class Case(ASTNodeWithChildren, ASTNodeWithExpression):
        pass
    cases : List[Case]

    def __init__(self):
        self.cases = []

    def walk_children(self, f):
        for case in self.cases:
            for child in case.children:
                f(child)

    def to_str(self, indent):
        def is_char(child):
            ts = child.token_str()
            return child.token.category == Token.Category.STRING_LITERAL and (len(ts) == 3 or (ts[:2] == '"\\' and len(ts) == 4))

        def char_if_len_1(child):
            if is_char(child):
                return "u'" + child.token.value(source)[1:-1].replace("'", R"\'") + "'"
            return child.to_str()

        r = ' ' * (indent*4) + 'switch (' + self.expression.to_str() + ")\n" + ' ' * (indent*4) + "{\n"
        for case in self.cases:
            r += ' ' * (indent*4) + ('default' if case.expression.token_str() in ('E', 'И', 'else', 'иначе') else 'case ' + char_if_len_1(case.expression)) + ":\n"
            for c in case.children:
                r += c.to_str(indent+1)
            r += ' ' * ((indent+1)*4) + "break;\n"
        return r + ' ' * (indent*4) + "}\n"

break_label_index = -1

class ASTLoop(ASTNodeWithChildren, ASTNodeWithExpression):
    loop_variable : str = None
    there_is_loop_break_inside_switch = -1

    def to_str(self, indent):
        if self.loop_variable != None:
            r = self.children_to_str_detect_single_stmt(indent, 'for (auto ' + self.loop_variable + ' : ' + self.expression.to_str() + ')')
        else:
            r = self.children_to_str_detect_single_stmt(indent, 'while (' + (self.expression.to_str() if self.expression != None else 'true') + ')')
        if self.there_is_loop_break_inside_switch != -1:
            r += ' ' * (indent*4) + 'break_' + ('' if self.there_is_loop_break_inside_switch == 0 else str(self.there_is_loop_break_inside_switch)) + ":\n"
        return r

    def walk_expressions(self, f):
        if self.expression != None: f(self.expression)

class ASTContinue(ASTNode):
    def to_str(self, indent):
        return ' ' * (indent*4) + "continue;\n"

class ASTLoopBreak(ASTNode):
    def to_str(self, indent):
        n = self.parent
        while True:
            if type(n) == ASTSwitch:
                n = n.parent
                while True:
                    if type(n) == ASTLoop:
                        if n.there_is_loop_break_inside_switch == -1:
                            global break_label_index
                            break_label_index += 1
                            n.there_is_loop_break_inside_switch = break_label_index
                        return ' ' * (indent*4) + 'goto break_' + ('' if n.there_is_loop_break_inside_switch == 0 else str(n.there_is_loop_break_inside_switch)) + ";\n"
                    n = n.parent
            if type(n) == ASTLoop:
                break
            n = n.parent
        return ' ' * (indent*4) + "break;\n"

class ASTReturn(ASTNodeWithExpression):
    def to_str(self, indent):
        return ' ' * (indent*4) + 'return' + (' ' + self.expression.to_str() if self.expression != None else '') + ";\n"

    def walk_expressions(self, f):
        if self.expression != None: f(self.expression)

class ASTException(ASTNodeWithExpression):
    def to_str(self, indent):
        return ' ' * (indent*4) + 'throw ' + self.expression.to_str() + ";\n"

class ASTExceptionTry(ASTNodeWithChildren):
    def to_str(self, indent):
        return self.children_to_str(indent, 'try')

class ASTExceptionCatch(ASTNodeWithChildren):
    exception_object_type : str
    exception_object_name : str = ''

    def to_str(self, indent):
        return self.children_to_str(indent, 'catch (const ' + self.exception_object_type + '&' + (' ' + self.exception_object_name if self.exception_object_name != '' else '') + ')')

class ASTTypeDefinition(ASTNodeWithChildren):
    base_types : List[str]
    type_name : str
    constructors : List[ASTFunctionDefinition]

    def __init__(self, constructors = None):
        super().__init__()
        self.constructors = constructors or []

    def to_str(self, indent):
        r = ''
        if self.tokeni > 0:
            ti = self.tokeni - 1
            while ti > 0 and tokens[ti].category in (Token.Category.SCOPE_END, Token.Category.STATEMENT_SEPARATOR):
                ti -= 1
            r = (source[tokens[ti].end:tokens[self.tokeni].start].count("\n")-1) * "\n"
        r += ' ' * (indent*4) \
          + 'class ' + self.type_name + (' : ' + ', '.join(map(lambda c: 'public ' + c, self.base_types)) if len(self.base_types) else '') \
          + "\n" + ' ' * (indent*4) + "{\n" + ' ' * (indent*4) + "public:\n"
        for c in self.children:
            r += c.to_str(indent+1)
        return r + ' ' * (indent*4) + "};\n"

class ASTMain(ASTNodeWithChildren):
    found_reference_to_argv = False

    def to_str(self, indent):
        if importing_module:
            return ''
        if not self.found_reference_to_argv:
            return self.children_to_str(indent, 'int main()')
        return self.children_to_str(indent, 'int MAIN_WITH_ARGV()', add_at_beginning = ' ' * ((indent+1)*4) + "INIT_ARGV();\n\n")

def next_token():
    global token, tokeni, tokensn
    if token == None and tokeni != -1:
        raise Error('no more tokens', Token(len(source), len(source), Token.Category.STATEMENT_SEPARATOR))
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
        raise Error('expected ' + value, token)
    next_token()

def peek_token(how_much = 1):
    return tokens[tokeni+how_much] if tokeni+how_much < len(tokens) else Token()

# This implementation is based on [http://svn.effbot.org/public/stuff/sandbox/topdown/tdop-4.py]
def expression(rbp = 0):
    def check_tokensn():
        if tokensn == None:
            raise Error('unexpected end of source', Token(len(source), len(source), Token.Category.STATEMENT_SEPARATOR))
        if tokensn.symbol == None:
            raise Error('no symbol corresponding to token `' + token.value(source) + '` (belonging to ' + str(token.category) +') found while parsing expression', token)
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

infix('[+]', 20); infix('->', 20)

infix('|', 30); infix('&', 40)

infix('==', 50); infix('!=', 50); infix('C', 50); infix('С', 50); infix('in', 50); infix('!C', 50); infix('!С', 50); infix('!in', 50)

infix('(concat)', 52) # `instr[prevci - 1 .< prevci]‘’prevc C ("/\\", "\\/")` = `(instr[prevci - 1 .< prevci]‘’prevc) C ("/\\", "\\/")`

infix('..', 55); infix('.<', 55); infix('<.', 55); infix('<.<', 55) # ch C ‘0’..‘9’ = ch C (‘0’..‘9’)
#postfix('..', 55)

infix('<', 60); infix('<=', 60)
infix('>', 60); infix('>=', 60)

infix('[|]', 70); infix('(+)', 80); infix('[&]', 90)

infix('<<', 100); infix('>>', 100)

infix('+', 110); infix('-', 110)

infix('*', 120); infix('/', 120); infix('I/', 120); infix('Ц/', 120)
infix('%', 120)

prefix('-', 130); prefix('+', 130); prefix('!', 130); prefix('(-)', 130); prefix('--', 130); prefix('++', 130)

infix_r('^', 140)

symbol('.', 150); symbol(':', 150); symbol('[', 150); symbol('(', 150); symbol(')'); symbol(']'); postfix('--', 150); postfix('++', 150)
prefix('.', 150); prefix(':', 150)

infix_r('=', 10); infix_r('+=', 10); infix_r('-=', 10); infix_r('*=', 10); infix_r('/=', 10); infix_r('I/=', 10); infix_r('Ц/=', 10); infix_r('%=', 10); infix_r('>>=', 10); infix_r('<<=', 10); infix_r('**=', 10)
infix_r('[+]=', 10); infix_r('[&]=', 10); infix_r('[|]=', 10); infix_r('(+)=', 10); infix_r('‘’=', 10)

symbol('(name)').nud = lambda self: self
symbol('(literal)').nud = lambda self: self
symbol('(constant)').nud = lambda self: self

symbol(';')
symbol(',')
symbol("',")

def led(self, left):
    self.append_child(left) # [(
    if token.value(source) not in (']', ')'):
        self.append_child(expression(self.symbol.led_bp))
    return self
symbol('..', 55).set_led_bp(55, led)

def led(self, left):
    if token.category != Token.Category.NAME:
        raise Error('expected an attribute name', token)
    self.append_child(left)
    self.append_child(tokensn)
    next_token()
    return self
symbol('.').led = led

class Module:
    scope : Scope
modules : Dict[str, Module] = {}

def led(self, left):
    if token.category != Token.Category.NAME:
        raise Error('expected an identifier name', token)

    # Process module [transpile it if necessary and load it]
    global scope
    module_name = left.to_str()
    if module_name not in modules:
        module_file_name = os.path.dirname(file_name) + '/' + module_name
        try:
            modulefstat = os.stat(module_file_name + '.11l')
        except FileNotFoundError:
            raise Error('can not import module `' + module_name + '`: file `' + module_file_name + '.11l` is not found', left.token)

        hpp_file_mtime = 0
        if os.path.isfile(module_file_name + '.hpp'):
            hpp_file_mtime = os.stat(module_file_name + '.hpp').st_mtime
        if hpp_file_mtime == 0 \
                or modulefstat.st_mtime       > hpp_file_mtime \
                or os.stat(__file__).st_mtime > hpp_file_mtime \
                or os.stat(os.path.dirname(__file__) + '/tokenizer.py').st_mtime > hpp_file_mtime \
                or True: # always parse module for a while
            module_source = open(module_file_name + '.11l', encoding = 'utf-8-sig').read()
            prev_scope = scope
            open(module_file_name + '.hpp', 'w', encoding = 'utf-8-sig', newline = "\n").write('namespace ' + module_name + "\n{\n" # utf-8-sig is for MSVC (fix of error C2015: too many characters in constant [`u'‘'`]) # ’
                + parse_and_to_str(tokenizer.tokenize(module_source), module_source, module_file_name + '.11l', True) + "}\n")
            modules[module_name] = Module()
            modules[module_name].scope = scope
            scope = prev_scope

    self.append_child(left)
    self.append_child(tokensn)
    next_token()
    return self
symbol(':').led = led

def led(self, left):
    self.function_call = True
    self.append_child(left) # (
    if token.value(source) != ')':
        while True:
            if token.category != Token.Category.STRING_LITERAL and token.value(source)[-1] == "'":
                self.append_child(tokensn)
                next_token()
                self.append_child(expression())
            else:
                self.children.append(None)
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
        raise Error('expected a new scope (indented block or opening curly bracket)', token)
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
                    raise Error('expected end of scope (dedented block or closing curly bracket)', token)
                next_token()
            else:
                self.append_child(expression())
        else:
            self.append_child(expression())
            advance_scope_begin()
            self.append_child(expression())
            if token.category != Token.Category.SCOPE_END:
                raise Error('expected end of scope (dedented block or closing curly bracket)', token)
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
        raise Error('expected end of scope (dedented block or closing curly bracket)', token)
    next_token()
    if not token.value(source) in ('E', 'И', 'else', 'иначе'):
        raise Error('expected else block', token)
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
            raise Error('expected ' + what_name, token)
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
                elif token.value(source) == '(': # this is constructor [`F () {...}` or `F (...) {...}`] or operator() [`F ()(...) {...}`]
                    if peek_token().value(source) == ')' and peek_token(2).value(source) == '(': # ) # this is operator()
                        next_token()
                        next_token()
                        node.function_name = '()'
                    else:
                        node.function_name = ''
                        if type(this_node) == ASTTypeDefinition:
                            this_node.constructors.append(node)
                elif token.category == Token.Category.OPERATOR:
                    node.function_name = token.value(source)
                    next_token()
                else:
                    raise Error('incorrect function name', token)

                if token.value(source) != '(': # )
                    raise Error('expected `(` after function name', token) # )(

                next_token()
                was_default_argument = False
                while token.value(source) != ')':
                    if token.value(source) == "',":
                        assert(node.first_named_only_argument == None)
                        node.first_named_only_argument = len(node.function_arguments)
                        next_token()
                        continue
                    type_ = ''
                    if token.value(source)[0].isupper(): # this is a type name
                        type_ = token.value(source)
                        next_token()
                    qualifiers = ''
                    if token.value(source) == '=':
                        qualifiers += '='
                        next_token()
                    if token.category != Token.Category.NAME:
                        raise Error('expected function\'s argument name', token)
                    func_arg_name = token.value(source)
                    next_token()
                    if token.value(source) == '=':
                        next_token()
                        default = expression().to_str()
                        was_default_argument = True
                    else:
                        if was_default_argument and node.first_named_only_argument == None:
                            raise Error('non-default argument follows default argument', tokens[tokeni-1])
                        default = ''
                    node.function_arguments.append((func_arg_name, default, type_, qualifiers)) # ((
                    if token.value(source) not in ',)':
                        raise Error('expected `,` or `)` in function\'s arguments list', token)
                    if token.value(source) == ',':
                        next_token()

                node.last_non_default_argument = len(node.function_arguments) - 1
                while node.last_non_default_argument >= 0 and node.function_arguments[node.last_non_default_argument][1] != '':
                    node.last_non_default_argument -= 1

                scope.add_function(node.function_name, node)

                next_token()
                if token.value(source) == '->':
                    next_token()
                    node.function_return_type = token.value(source)
                    next_token()

                new_scope(node, map(lambda arg: (arg[0], arg[2]), node.function_arguments))

            elif token.value(source) in ('T', 'Т', 'type', 'тип'):
                node = ASTTypeDefinition()
                node.type_name = expected_name('type name')
                scope.add_name(node.type_name, node)
                node.base_types = []

                if token.value(source) == '(':
                    while True:
                        node.base_types.append(expected_name('base type name'))
                        if token.value(source) != ',':
                            break
                    if token.value(source) != ')': # (
                        raise Error('expected `)`', token)
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
                        n.else_or_elif.parent = n
                        n = n.else_or_elif
                        next_token()
                        next_token()
                        n.set_expression(expression())
                        new_scope(n)
                    if token != None and token.value(source) in ('E', 'И', 'else', 'иначе') and not peek_token().value(source) in ('I', 'Е', 'if', 'если'):
                        n.else_or_elif = ASTElse()
                        n.else_or_elif.parent = n
                        next_token()
                        new_scope(n.else_or_elif)
                        break

            elif token.value(source) in ('S', 'В', 'switch', 'выбрать'):
                node = ASTSwitch()
                next_token()
                node.set_expression(expression())

                advance_scope_begin()
                while token.category != Token.Category.SCOPE_END:
                    case = ASTSwitch.Case()
                    case.parent = node
                    if token.value(source) in ('E', 'И', 'else', 'иначе'):
                        case.set_expression(tokensn)
                        next_token()
                    else:
                        case.set_expression(expression())
                    new_scope(case)
                    node.cases.append(case)
                next_token()

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

            elif token.value(source) in ('L.continue', 'Ц.продолжить', 'loop.continue', 'цикл.продолжить'):
                node = ASTContinue()
                next_token()
                if token != None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            elif token.value(source) in ('L.break', 'Ц.прервать', 'loop.break', 'цикл.прервать'):
                node = ASTLoopBreak()
                next_token()
                if token != None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            elif token.value(source) in ('R', 'Р', 'return', 'вернуть'):
                node = ASTReturn()
                next_token()
                if token.category in (Token.Category.SCOPE_END, Token.Category.STATEMENT_SEPARATOR):
                    node.expression = None
                else:
                    node.set_expression(expression())
                if token != None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            elif token.value(source) in ('X', 'Х', 'exception', 'исключение'):
                node = ASTException()
                next_token()
                node.set_expression(expression())
                if token != None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            elif token.value(source) in ('X.try', 'Х.контроль', 'exception.try', 'исключение.контроль'):
                node = ASTExceptionTry()
                next_token()
                new_scope(node)

            elif token.value(source) in ('X.catch', 'Х.перехват', 'exception.catch', 'исключение.перехват'):
                node = ASTExceptionCatch()
                node.exception_object_type = expected_name('exception object type name')
                if token.category == Token.Category.NAME:
                    node.exception_object_name = token.value(source)
                    next_token()
                new_scope(node)

            else:
                raise Error('unrecognized statement started with keyword', token)

        elif token.category == Token.Category.SCOPE_END:
            next_token()
            if token.category == Token.Category.STATEMENT_SEPARATOR and token.end == len(source): # Token.Category.EOF
                next_token()
                assert(token == None)
            return

        elif token.category == Token.Category.STATEMENT_SEPARATOR: # this `if` was added in revision 105 in order to support `hor_col_align = S instr[j .< j + 2] {‘<<’ {‘left’}; ‘>>’ {‘right’}; ‘><’ {‘center’}; ‘<>’ {‘justify’}}` [there was no STATEMENT_SEPARATOR after this line of code]
            next_token()
            assert(token.category != Token.Category.STATEMENT_SEPARATOR)
            continue

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
                    if node_expression.is_dict:
                        assert(len(node_expression.children) == 1)
                        node.type = 'Dict'
                        node.type_args = [node_expression.children[0].children[0].to_str(), node_expression.children[0].children[1].to_str()]
                    elif node_expression.is_list:
                        assert(len(node_expression.children) == 1)
                        node.type = 'Array'
                        node.type_args = [node_expression.children[0].to_str()]
                    else:
                        assert(node_expression.is_type)
                        node.type = node_expression.children[0].token.value(source)
                        for i in range(1, len(node_expression.children)):
                            node.type_args.append(node_expression.children[i].to_str())
                elif node.type == '(': # )
                    node.function_pointer = True
                    for i in range(len(node_expression.children)):
                        child = node_expression.children[i]
                        if child.token.category == Token.Category.NAME:
                            node.type_args.append(child.token_str())
                        else:
                            assert(child.symbol.id == '->' and i == len(node_expression.children) - 1)
                            assert(child.children[0].token.category == Token.Category.NAME)
                            assert(child.children[1].token.category == Token.Category.NAME or child.children[1].token_str() in ('N', 'Н', 'null', 'нуль'))
                            node.type_args.append(child.children[0].token_str())
                            node.type = child.children[1].token_str() # return value is the last
                assert(node.type[0].isupper() or node.type in ('var', 'перем')) # type name must starts with an upper case letter
                scope.add_name(node.var, node)
            else:
                node = ASTExpression()
                node.set_expression(node_expression)
            if not (token == None or token.category in (Token.Category.STATEMENT_SEPARATOR, Token.Category.SCOPE_END)):
                raise Error('expected end of statement', token)
            if token != None and token.category == Token.Category.STATEMENT_SEPARATOR:
                next_token()

        node.parent = this_node
        this_node.children.append(node)

    return

tokens    = []
source    = ''
tokeni    = -1
token     = Token(0, 0, Token.Category.STATEMENT_SEPARATOR)
scope     = Scope(None)
tokensn   = SymbolNode(token)
file_name = ''
importing_module = False

def parse_and_to_str(tokens_, source_, file_name_, importing_module_ = False, suppress_error_please_wrap_in_copy = False): # option suppress_error_please_wrap_in_copy is needed to simplify conversion of large Python source into C++
    if len(tokens_) == 0: return ASTProgram()
    global tokens, source, tokeni, token, break_label_index, scope, tokensn, file_name, importing_module, modules
    prev_tokens    = tokens
    prev_source    = source
    prev_tokeni    = tokeni
    prev_token     = token
#   prev_scope     = scope
    prev_tokensn   = tokensn
    prev_file_name = file_name
    prev_importing_module = importing_module
    prev_break_label_index = break_label_index
    tokens = tokens_ + [Token(len(source_), len(source_), Token.Category.STATEMENT_SEPARATOR)]
    source = source_
    tokeni = -1
    token = None
    break_label_index = -1
    scope = Scope(None)
    scope.add_function('print', ASTFunctionDefinition([('object', None, ''), ('end', SymbolNode(Token(0, 0, Token.Category.STRING_LITERAL), R'"\n"').to_str(), 'String'), ('flush', SymbolNode(Token(0, 0, Token.Category.CONSTANT), '0B').to_str())]))
    scope.add_function('assert', ASTFunctionDefinition([('expression', None, 'Bool'), ('message', SymbolNode(Token(0, 0, Token.Category.STRING_LITERAL), '‘’').to_str(), 'String')]))
    scope.add_function('exit', ASTFunctionDefinition([('arg', None, '')]))
    scope.add_function('zip', ASTFunctionDefinition([('iterable1', None, ''), ('iterable2', None, '')]))
    scope.add_function('sum', ASTFunctionDefinition([('iterable', None, '')]))
    scope.add_function('min', ASTFunctionDefinition([('object1', None, ''), ('object2', None, '')]))
    scope.add_function('max', ASTFunctionDefinition([('object1', None, ''), ('object2', None, '')]))
    scope.add_function('hex', ASTFunctionDefinition([('object', None, '')]))
    scope.add_name('Char', ASTTypeDefinition([ASTFunctionDefinition([('code', None)])]))
    scope.add_name('File', ASTTypeDefinition([ASTFunctionDefinition([('name', None, 'String'), ('mode', SymbolNode(Token(0, 0, Token.Category.STRING_LITERAL), '‘r’').to_str(), 'String'), ('encoding', SymbolNode(Token(0, 0, Token.Category.STRING_LITERAL), '‘utf-8’').to_str(), 'String'), ('newline', SymbolNode(Token(0, 0, Token.Category.STRING_LITERAL), '‘’').to_str(), 'String')])]))
    for type_ in cpp_type_from_11l:
        scope.add_name(type_, ASTTypeDefinition([ASTFunctionDefinition([('object', None, '')])]))
    file_name = file_name_
    importing_module = importing_module_
    if not importing_module:
        modules = {}
    next_token()
    p = ASTProgram()
    parse_internal(p)

    if len(modules):
        p.beginning_extra = "\n".join(map(lambda m: '#include "' + m + '.hpp"', modules)) + "\n\n"

    found_reference_to_argv = False
    def find_reference_to_argv(node):
        def f(e : SymbolNode):
            if len(e.children) == 1 and e.symbol.id == ':' and e.children[0].token_str() == 'argv':
                nonlocal found_reference_to_argv
                found_reference_to_argv = True
                return
            for child in e.children:
                if child != None:
                    f(child)

        node.walk_expressions(f)
        node.walk_children(find_reference_to_argv)
    find_reference_to_argv(p)

    if found_reference_to_argv:
        assert(type(p.children[-1]) == ASTMain)
        p.children[-1].found_reference_to_argv = True
        p.beginning_extra += "Array<String> argv;\n\n"

    s = p.to_str() # call `to_str()` moved here [from outside] because it accesses global variables `source` (via `token.value(source)`) and `tokens` (via `tokens[ti]`)

    tokens    = prev_tokens
    source    = prev_source
    tokeni    = prev_tokeni
    token     = prev_token
#   scope     = prev_scope
    tokensn   = prev_tokensn
    file_name = prev_file_name
    importing_module = prev_importing_module
    break_label_index = prev_break_label_index

    return s
