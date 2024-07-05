try:
    from tokenizer import Token
    import tokenizer
except ImportError:
    from .tokenizer import Token
    from . import tokenizer
from typing import List, Tuple, Dict, Callable, Set, ClassVar
from enum import IntEnum
import os, re, eldf

class Error(Exception):
    def __init__(self, message, token):
        self.message = message
        self.pos = token.start
        self.end = token.end

class Scope:
    parent : 'Scope'
    node : 'ASTNode' = None

    class Id:
        type : str
        default_val: str
        type_node : 'ASTTypeDefinition' = None
        ast_nodes : List['ASTNodeWithChildren']
        last_occurrence : 'SymbolNode' = None

        def __init__(self, type, default_val = '', ast_node = None):
            assert(type is not None)
            self.type = type
            self.default_val = default_val
            self.ast_nodes = []
            if ast_node is not None:
                self.ast_nodes.append(ast_node)

        def init_type_node(self, scope):
            if self.type != '':
                tid = scope.find(self.type)
                if tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) == ASTTypeDefinition:
                    self.type_node = tid.ast_nodes[0]

        def serialize_to_dict(self):
            ast_nodes = []
            for ast_node in self.ast_nodes:
                if type(ast_node) in (ASTFunctionDefinition, ASTTypeDefinition):
                    ast_nodes.append(ast_node.serialize_to_dict())
            return {'type': self.type, 'ast_nodes': ast_nodes}

        def deserialize_from_dict(self, d):
            #self.type = d['type']
            for ast_node_dict in d['ast_nodes']:
                ast_node = ASTFunctionDefinition() if ast_node_dict['node_type'] == 'function' else ASTTypeDefinition()
                ast_node.deserialize_from_dict(ast_node_dict)
                self.ast_nodes.append(ast_node)

    ids : Dict[str, Id]
    is_function : bool
    is_lambda = False

    def __init__(self, func_args):
        self.parent = None
        if func_args is not None:
            self.is_function = True
            self.ids = dict(map(lambda x: (x[0], Scope.Id(x[1], x[2])), func_args))
        else:
            self.is_function = False
            self.ids = {}
        self.module_aliases = {}

    def init_ids_type_node(self):
        for id in self.ids.values():
            id.init_type_node(self.parent)

    def serialize_to_dict(self):
        ids_dict = {}
        for name, id in self.ids.items():
            ids_dict[name] = id.serialize_to_dict()
        return ids_dict

    def deserialize_from_dict(self, d):
        for name, id_dict in d.items():
            id = Scope.Id(id_dict['type'])
            id.deserialize_from_dict(id_dict)
            self.ids[name] = id

    def find_in_current_function(self, name):
        s = self
        while True:
            if name in s.ids:
                return s.ids[name]
            if s.is_function:
                return None
            s = s.parent
            if s is None:
                return None

    def find_in_current_type_function(self, name):
        s = self
        while True:
            if name in s.ids:
                return True
            if s.is_function and type(s.node) == ASTFunctionDefinition and type(s.node.parent) == ASTTypeDefinition:
                return False
            s = s.parent
            if s is None:
                return False

    def find(self, name):
        s = self
        while True:
            id = s.ids.get(name)
            if id is not None:
                return id
            s = s.parent
            if s is None:
                return None

    def module_alias(self, name):
        s = self
        while True:
            ma = s.module_aliases.get(name)
            if ma is not None:
                return ma
            s = s.parent
            if s is None:
                return None

    def find_and_return_scope(self, name):
        s = self
        if type(s.node) == ASTTypeDefinition:
            id = s.ids.get(name)
            if id is not None:
                return id, s
        while True:
            if type(s.node) != ASTTypeDefinition:
                id = s.ids.get(name)
                if id is not None:
                    return id, s
            s = s.parent
            if s is None:
                return None, None

    def add_function(self, name, ast_node):
        if name in self.ids:                                                   # V &id = .ids.set_if_not_present(name, .Id(‘’)) // [[[or `put_if_absent` as in Java, or `add_if_absent`]]] note that this is an error: `V id = .ids.set_if_not_present(...)`, but you can do this: `V id = copy(.ids.set_if_not_present(...))`
            assert(type(self.ids[name].ast_nodes[0]) == ASTFunctionDefinition) # assert(id.ast_nodes.empty | T(id.ast_nodes[0]) == ASTFunctionDefinition)
            self.ids[name].ast_nodes.append(ast_node)                          # id.ast_nodes [+]= ast_node
        else:
            self.ids[name] = Scope.Id('', '', ast_node)

    def add_name(self, name, ast_node):
        if name in self.ids:                                                            # I !.ids.set(name, .Id(‘’, ast_node))
            if isinstance(ast_node, ASTVariableDeclaration):
                t = ast_node.type_token
            elif isinstance(ast_node, ASTNodeWithChildren):
                t = tokens[ast_node.tokeni + 1]
            else:
                t = token
            raise Error('redefinition of already defined identifier is not allowed', t) #    X Error(‘redefinition ...’, ...)
        if type(ast_node) == ASTTypeDefinition:
            ast_node.type_name = name
        self.ids[name] = Scope.Id('', '', ast_node)

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

int_is_int64 = False
python_division = False

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
    is_var_decl    = False
    scope : Scope
    token_str_override : str

    def __init__(self, token, token_str_override = None, symbol = None):
        self.token = token
        self.children = []
        self.scope = scope
        self.token_str_override = token_str_override
        self.symbol = symbol

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
            if len(self.children) == 0:
                return self.token.end + 1
            return (self.children[-1] or self.children[-2]).rightmost() + 1

        return self.children[-1].rightmost()

    def left_to_right_token(self):
        return Token(self.leftmost(), self.rightmost(), Token.Category.NAME)

    def token_str(self):
        return self.token.value(source) if not self.token_str_override else self.token_str_override

    def to_debug_str(self):
        if self.token.category in (Token.Category.NAME, Token.Category.NUMERIC_LITERAL):
            return self.token_str()

        if self.function_call:
            res = self.children[0].to_debug_str() + '('
            for i in range(1, len(self.children), 2):
                res += self.children[i+1].to_debug_str()
                if i < len(self.children)-2:
                    res += ', '
            return res + ')'

        if len(self.children) == 1:
            if self.symbol.id == '(': # )
                return '(' + self.children[0].to_debug_str() + ')'
            if self.postfix:
                return '(' + self.children[0].to_debug_str() + self.symbol.id + ')'
            else:
                return '(' + self.symbol.id + self.children[0].to_debug_str() + ')'

        if len(self.children) == 3: # "with"-expression
            assert(self.children[1].token.category == Token.Category.SCOPE_BEGIN)
            return self.children[0].to_debug_str() + '. (' + self.children[2].to_debug_str() + ')'

        assert(len(self.children) == 2)
        return '(' + self.children[0].to_debug_str() + ' ' + self.symbol.id + ' ' + self.children[1].to_debug_str() + ')'

    def to_type_str(self):
        if self.symbol.id == '[': # ]
            if self.is_list:
                assert(len(self.children) == 1)
                return 'Array[' + self.children[0].to_type_str() + ']'
            elif self.is_dict:
                assert(len(self.children) == 1 and self.children[0].symbol.id == '=')
                return 'Dict[' + self.children[0].children[0].to_type_str() + ', ' \
                               + self.children[0].children[1].to_type_str() + ']'
            else:
                assert(self.is_type)
                r = self.children[0].token.value(source) + '['
                for i in range(1, len(self.children)):
                    r += self.children[i].to_type_str()
                    if i < len(self.children) - 1:
                        r += ', '
                return r + ']'
        elif self.symbol.id == '(': # )
            if len(self.children) == 1 and self.children[0].symbol.id == '->':
                r = 'Callable['
                c0 = self.children[0]
                if c0.children[0].symbol.id == '(': # )
                    for child in c0.children[0].children:
                        r += child.to_type_str() + ', '
                else:
                    r += c0.children[0].to_type_str() + ', '
                return r + c0.children[1].to_type_str() + ']'
            elif self.function_call:
                assert(self.children[0].token_str() in ('T', 'Т', 'type', 'тип'))
                return 'TYPE_OF(' + self.children[2].to_str() + ')'
            else:
                assert(self.tuple)
                r = '('
                for i in range(len(self.children)):
                    assert(self.children[i].symbol.id != '->')
                    r += self.children[i].to_type_str()
                    if i < len(self.children) - 1:
                        r += ', '
                return r + ')'
        elif self.symbol.id == 'T? ':
            return self.children[0].to_type_str() + '?'
        elif self.symbol.id == ':':
            return self.children[0].to_type_str() + ':' + self.children[1].to_type_str()

        assert(self.token.category == Token.Category.NAME or (self.token.category == Token.Category.CONSTANT and self.token.value(source) in ('N', 'Н', 'null', 'нуль')))
        return self.token_str()

    inside_it_lambda: ClassVar = False

    def to_str(self):
        if not SymbolNode.inside_it_lambda:
            found_it = False
            def f(sn: SymbolNode):
                if sn.function_call:
                    return
                if sn.token.category == Token.Category.NAME and sn.token_str() == '(->)':
                    nonlocal found_it
                    found_it = True
                else:
                    for child in sn.children:
                        if child is not None:
                            f(child)
            f(self)
            if found_it:
                SymbolNode.inside_it_lambda = True
                r = '[](const auto &it){return ' + self.to_str() + ';}'
                SymbolNode.inside_it_lambda = False
                return r

        if self.token.category == Token.Category.NAME:
            if self.token_str() in ('L.index', 'Ц.индекс', 'loop.index', 'цикл.индекс'):
                parent = self
                while parent.parent:
                    parent = parent.parent
                ast_parent = parent.ast_parent
                while True:
                    if type(ast_parent) == ASTLoop:
                        ast_parent.has_L_index = True
                        break
                    ast_parent = ast_parent.parent
                return 'Lindex'

            if self.token_str() == '(.)':
                # if self.parent is not None and self.parent.symbol.id == '=' and self is self.parent.children[1]: # `... = (.)` -> `... = this;`
                #     return 'this'
                return '*this'

            if self.token_str() == '(->)':
                return 'it'

            tid = self.scope.find(self.token_str())
            if tid is not None and ((len(tid.ast_nodes) and isinstance(tid.ast_nodes[0], ASTVariableDeclaration) and tid.ast_nodes[0].is_ptr and not tid.ast_nodes[0].nullable) # `animals [+]= animal` -> `animals.append(std::move(animal));`
                                 or (tid.type_node is not None and (tid.type_node.has_virtual_functions or tid.type_node.has_pointers_to_the_same_type))) \
                                and (self.parent is None or self.parent.symbol.id not in ('.', ':', '!')):
                if tid.last_occurrence is None:
                    last_reference = None
                    var_name = self.token_str()
                    def find_last_reference_to_identifier(node):
                        def f(e : SymbolNode):
                            if e.token.category == Token.Category.NAME and e.token_str() == var_name and id(e.scope.find(var_name)) == id(tid):
                                nonlocal last_reference
                                last_reference = e
                            for child in e.children:
                                if child is not None:
                                    f(child)
                        node.walk_expressions(f)
                        node.walk_children(find_last_reference_to_identifier)

                    if tid.type_node is not None:
                        find_last_reference_to_identifier(self.scope.node)
                        tid.last_occurrence = last_reference
                    else:
                        for index in range(len(tid.ast_nodes[0].parent.children)):
                            if id(tid.ast_nodes[0].parent.children[index]) == id(tid.ast_nodes[0]):
                                for index in range(index + 1, len(tid.ast_nodes[0].parent.children)):
                                    find_last_reference_to_identifier(tid.ast_nodes[0].parent.children[index])
                                tid.last_occurrence = last_reference
                                break

                if id(tid.last_occurrence) == id(self):
                    return 'std::move(' + self.token_str() + ')'

            if tid is not None and len(tid.ast_nodes) and isinstance(tid.ast_nodes[0], ASTVariableDeclaration) and tid.ast_nodes[0].type.endswith('?'):
                if self.parent is None or (not (self.parent.symbol.id in ('==', '!=') and self.parent.children[1].token_str() in ('N', 'Н', 'null', 'нуль'))
                                       and not (self.parent.symbol.id == '.')
                                       and not (self.parent.symbol.id == '?')
                                       and not (self.parent.symbol.id == '!' and self.parent.postfix)
                                       and not (self.parent.symbol.id == '=' and self is self.parent.children[0])):
                    return '*' + self.token_str()

            if self.token_str().startswith('@:'):
                var_name = self.token_str()[self.token_str().index(':') + 1:]
                tid = self.scope.find(var_name)
                if tid is not None and len(tid.ast_nodes) and isinstance(tid.ast_nodes[0], ASTVariableDeclaration) and tid.ast_nodes[0].is_static:
                    return 's_' + var_name

            return self.token_str().lstrip('@=').replace(':', '::')

        if self.token.category == Token.Category.KEYWORD and self.token_str() in ('L.last_iteration', 'Ц.последняя_итерация', 'loop.last_iteration', 'цикл.последняя_итерация'):
            parent = self
            while parent.parent:
                parent = parent.parent
            ast_parent = parent.ast_parent
            while True:
                if type(ast_parent) == ASTLoop:
                    ast_parent.has_L_last_iteration = True
                    break
                ast_parent = ast_parent.parent
            return '(__begin == __end)'

        if self.token.category == Token.Category.NUMERIC_LITERAL:
            n = self.token_str()
            if n[-1] in 'oо':
                return '0' + n[:-1] + 'LL'*int_is_int64
            if n[-1] in 'bд':
                return '0b' + n[:-1] + 'LL'*int_is_int64
            if n[-1] in 'LД':
                return n[:-1] + 'LL'
            if n[-1] == 's':
                return n[:-1] + 'f'
            if n[4:5] == "'" or n[-3:-2] == "'" or n[-2:-1] == "'" or n[-9:-8] == "'":
                nn = ''
                for c in n:
                    nn += {'А':'A','Б':'B','С':'C','Д':'D','Е':'E','Ф':'F'}.get(c, c)
                if n[-2:-1] == "'":
                    nn = nn.replace("'", '')
                return '0x' + nn
            if '.' in n or 'e' in n:
                return n
            return n + 'LL'*int_is_int64

        if self.token.category == Token.Category.STRING_LITERAL:
            s = self.token_str()
            indented = False
            if s[0] == '|':
                indented = True
                s = s[1:]
                assert(s[0] != '"')
            elif s.startswith(r'\/'):
                if s[3] != "\n":
                    raise Error('Zero indented multi-line string literal must start with a new line', Token(self.token.start + 3, self.token.start + 3, Token.Category.STRING_LITERAL))
                s = s[2] + s[4:]
                assert(s[0] != '"')
            if s[0] == '"':
                return 'u' + s + '_S'

            eat_left = 0
            while s[eat_left] == "'":
                eat_left += 1
            eat_right = 0
            while s[-1-eat_right] == "'":
                eat_right += 1
            s = s[1+eat_left*2:-1-eat_right*2]

            if indented:
                line_start = source.rfind("\n", 0, self.token.start) + 1
                indentation = ''
                for i in range(line_start, self.token.start):
                    indentation += "\t" if source[i] == "\t" else ' '
                indentation += ' ' * (2+eat_left*2)

                newline_pos = s.find("\n")
                if newline_pos != -1:
                    newline_pos += 1
                    new_s = s[:newline_pos]

                    while True:
                        if s[newline_pos:newline_pos+1] == "\n":
                            new_s += "\n"
                            newline_pos += 1
                            continue

                        if s[newline_pos:newline_pos+len(indentation)] != indentation:
                            error_pos = self.token.start + (2+eat_left*2) + newline_pos
                            raise Error('incorrect indentation of line in indented multi-line string literal', Token(error_pos, error_pos, Token.Category.STRING_LITERAL))

                        prev_newline_pos = newline_pos
                        newline_pos = s.find("\n", newline_pos)
                        if newline_pos == -1:
                            new_s += s[prev_newline_pos + len(indentation):]
                            break
                        newline_pos += 1
                        new_s += s[prev_newline_pos + len(indentation):newline_pos]

                    s = new_s

            if '\\' in s or "\n" in s:
                delimiter = '' # (
                while ')' + delimiter + '"' in s:
                    delimiter += "'"
                return 'uR"' + delimiter + '(' + s + ')' + delimiter + '"_S'

            return 'u"' + repr(s)[1:-1].replace('"', R'\"').replace(R"\'", "'") + '"_S'

        if self.token.category == Token.Category.FSTRING:
            nfmtstr = ''
            format_args = ''
            i = 0
            while i < len(self.children):
                child = self.children[i]
                if child.token.category == Token.Category.STRING_LITERAL:
                    s = child.token.value(source)
                    j = 0
                    while j < len(s):
                        if s[j] == '#' and (s[j+1:j+2] in ('#', '.', '<') or s[j+1:j+2].isdigit()):
                            nfmtstr += '##'
                        elif s[j] == '{':
                            j += 1
                            assert(s[j] == '{')
                            nfmtstr += '{'
                        elif s[j] == '}':
                            j += 1
                            assert(s[j] == '}')
                            nfmtstr += '}'
                        else:
                            nfmtstr += s[j]
                        j += 1
                else:
                    nfmtstr += '#'
                    if i + 1 < len(self.children) and self.children[i + 1].token.category == Token.Category.STATEMENT_SEPARATOR:
                        fmt = self.children[i + 1].token.value(source)#.lstrip(' ')
                        if fmt[0] == ' ':
                            p = self.children[i + 1].token.start
                            raise Error('space format option is not supported [but you can write `f:‘ {c:4}’` instead of `f:‘{c: 5}’`]', Token(p, p, Token.Category.DELIMITER))
                        if fmt[0] == '<':
                            nfmtstr += '<'
                            fmt = fmt[1:]
                        if '.' in fmt:
                            if fmt[0] == '0' and fmt[1:2].isdigit(): # zero padding
                                nfmtstr += '0'
                            (before_period, after_period) = map(int, ('0' + fmt).split('.')) # `'0' + ` is for e.g. `#.6` (fmt = '.6')
                            b = before_period
                            if after_period != 0:
                                b -= after_period + 1
                            if b > 1:
                                nfmtstr += str(b)
                            nfmtstr += '.' + str(after_period)
                        else:
                            nfmtstr += fmt
                        i += 1
                    else:
                        nfmtstr += '.'

                    if format_args != '':
                        format_args += ', '
                    format_args += child.to_str()
                i += 1

            if source[self.token.start + 2] == '"':
                nfmtstr = 'u"' + nfmtstr + '"_S'
            elif '\\' in nfmtstr or "\n" in nfmtstr:
                delimiter = '' # (
                while ')' + delimiter + '"' in nfmtstr:
                    delimiter += "'"
                nfmtstr = 'uR"' + delimiter + '(' + nfmtstr + ')' + delimiter + '"_S'
            else:
                nfmtstr = 'u"' + repr(nfmtstr)[1:-1].replace('"', R'\"').replace(R"\'", "'") + '"_S'
            return nfmtstr + '.format(' + format_args + ')'

        if self.token.category == Token.Category.CONSTANT:
            return {'N': 'nullptr', 'Н': 'nullptr', 'null': 'nullptr', 'нуль': 'nullptr', '0B': 'false', '0В': 'false', '1B': 'true', '1В': 'true'}[self.token_str()]

        def is_char(child):
            ts = child.token_str()
            return child.token.category == Token.Category.STRING_LITERAL and (len(ts) == 3 or (ts[:2] == '"\\' and len(ts) == 4))

        def char_or_str(child, is_char):
            if is_char:
                if child.token_str()[1:-1] == "\\":
                    return R"u'\\'_C"
                return "u'" + child.token_str()[1:-1].replace("'", R"\'") + "'_C"
            return child.to_str()

        if self.symbol.id == '(': # )
            if self.function_call:
                func_name = self.children[0].to_str()
                f_node = None
                if self.children[0].symbol.id == '.':
                    if len(self.children[0].children) == 1:
                        in_with = False
                        n = self
                        while True:
                            n = n.parent
                            if n is None:
                                break
                            if len(n.children) == 3 and not n.function_call and n.children[1].token.category == Token.Category.SCOPE_BEGIN:
                                in_with = True
                                break
                        if not in_with:
                            s = self.scope
                            while True:
                                if s.is_function:
                                    if type(s.node) != ASTFunctionDefinition:
                                        assert(s.is_lambda)
                                        raise Error('probably `@` is missing (before this dot)', self.children[0].token)
                                    if type(s.node.parent) == ASTTypeDefinition:
                                        assert(s.node.parent.scope == s.parent)
                                        fid = s.node.parent.find_id_including_base_types(self.children[0].children[0].to_str())
                                        if fid is None:
                                            raise Error('call of undefined method `' + func_name + '`', self.children[0].children[0].token)
                                        if len(fid.ast_nodes) > 1:
                                            raise Error('methods\' overloading is not supported for now', self.children[0].children[0].token)
                                        f_node = fid.ast_nodes[0]
                                        if type(f_node) == ASTTypeDefinition:
                                            if len(f_node.constructors) == 0:
                                                f_node = ASTFunctionDefinition()
                                            else:
                                                if len(f_node.constructors) > 1:
                                                    raise Error('constructors\' overloading is not supported for now (see type `' + f_node.type_name + '`)', self.children[0].left_to_right_token())
                                                f_node = f_node.constructors[0]
                                    break
                                if type(s.node) == ASTWith:
                                    break
                                s = s.parent
                                assert(s)
                    elif func_name.endswith('.map') and self.children[2].token.category == Token.Category.NAME and self.children[2].token_str()[0].isupper():
                        c2 = self.children[2].to_str()
                        return func_name + '([](const auto &x){return ' + {'Int':'to_int', 'Int64':'to_int64', 'Long':'to_int64', 'UInt64':'to_uint64', 'ULong':'to_uint64', 'UInt32':'to_uint32', 'Float':'to_float', 'SFloat':'to_float32', 'Float32':'to_float32'}.get(c2, c2) + '(x);})'
                    elif func_name.endswith('.split'):
                        if len(self.children) == 5 and self.children[3] is not None and self.children[3].token_str() == "req'":
                            return func_name + '_req(' + self.children[2].to_str() + ', ' + self.children[4].to_str() + ')'
                        f_node = type_of(self.children[0])
                        if f_node is None: # assume this is String method
                            f_node = builtins_scope.find('String').ast_nodes[0].scope.ids.get('split').ast_nodes[0]
                    elif func_name.endswith('.read_bytes') and len(self.children) == 3 and self.children[1] is not None and self.children[1].token_str() == "at_most'":
                        return func_name + '_at_most(' + self.children[2].to_str() + ')'
                    elif func_name.endswith('.sort_range'):
                        f_node = type_of(self.children[0])
                        if f_node is None: # assume this is Array method
                            f_node = builtins_scope.find('Array').ast_nodes[0].scope.ids.get('sort_range').ast_nodes[0]
                    elif func_name.endswith('.pop') and len(self.children) == 3 and self.children[2].to_str().startswith('(len)'):
                        return func_name + '_plus_len(' + self.children[2].to_str()[len('(len)'):] + ')'
                    elif func_name.endswith(('.count', '.index')) and len(self.children) == 3 and self.children[2].token.category == Token.Category.STRING_LITERAL:
                        return func_name + '(' + char_or_str(self.children[2], is_char(self.children[2])) + ')'
                    elif self.children[0].children[1].token.value(source) == 'union':
                        func_name = self.children[0].children[0].to_str() + '.set_union'
                    elif func_name.endswith('.to_bytes'):
                        return 'bytes_from_int(' + self.children[0].children[0].to_str() + ')'
                    elif func_name.endswith('.bits'):
                        return 'int_bits(' + self.children[0].children[0].to_str() + ', ' + self.children[2].to_str() + ')'
                    elif func_name.endswith('.bit'):
                        return 'bit_ref(' + self.children[0].children[0].to_str() + ', ' + self.children[2].to_str() + ')'
                    elif func_name.endswith('.as'):
                        return trans_type(self.children[2].to_type_str(), self.children[2].scope, self.children[2].token) + '(' + self.children[0].children[0].to_str() + ')'
                    else:
                        f_node = type_of(self.children[0])
                elif func_name == 'Bool':
                    func_name = 'bool'
                elif func_name in ('Int', 'Int64', 'UInt64', 'Int32', 'UInt32', 'Int16', 'UInt16', 'Int8', 'Byte'):
                    if self.children[1] is not None and self.children[1].token_str() in ("bytes'", "bytes_be'"):
                        be = '_be' * (self.children[1].token_str() == "bytes_be'")
                        int_from_bytes = 'int_from_bytes' + be if func_name == 'Int' else 'int_t_from_bytes' + be + '<' + func_name + '>'
                        if self.children[2].symbol.id == '[' and not self.children[2].is_list and self.children[2].children[1].symbol.id in ('..', '.<', '.+', '<.', '<.<'): # ]
                            return int_from_bytes + '(' + self.children[2].children[0].to_str() + ', ' + self.children[2].children[1].to_str() + ')'
                        return int_from_bytes + '(' + self.children[2].to_str() + ')'
                    func_name = {'Int':'to_int', 'Int64':'to_int64', 'UInt64':'to_uint64', 'Int32':'to_int32', 'UInt32':'to_uint32', 'Int16':'to_int16', 'UInt16':'to_uint16', 'Int8':'to_int8', 'Byte':'Byte'}[func_name]
                    f_node = builtins_scope.find('Int').ast_nodes[0].constructors[0]
                elif func_name in ('Float', 'SFloat', 'Float32', 'Float64'):
                    func_name = 'to_float' + '32'*(func_name in ('SFloat', 'Float32'))
                elif func_name in cpp_vectype_from_11l:
                    func_name = cpp_vectype_from_11l[func_name]
                elif func_name == 'Char' and self.children[2].token.category == Token.Category.STRING_LITERAL:
                    assert(self.children[1] is None) # [-TODO: write a good error message-]
                    if not is_char(self.children[2]):
                        raise Error('Char can be constructed only from single character string literals', self.children[2].token)
                    return char_or_str(self.children[2], True)
                elif func_name == 'Char' and self.children[1] is not None and self.children[1].token_str() == "string'":
                    return 'Char(' + self.children[2].to_str() + ')'
                elif func_name == 'Char' and self.children[1] is not None and self.children[1].token_str() == "digit'":
                    return 'char_from_digit(' + self.children[2].to_str() + ')'
                elif func_name == 'Time' and len(self.children) == 3 and self.children[1] is not None and self.children[1].token_str() == "unix_time'":
                    return 'timens::from_unix_time(' + self.children[2].to_str() + ')'
                elif func_name == 'String' and self.children[2].token.category == Token.Category.STRING_LITERAL:
                    return self.children[2].to_str()
                elif func_name == 'Bytes' and self.children[2].token.category == Token.Category.STRING_LITERAL:
                    return '"' + self.children[2].token_str()[1:-1] + '"_B'
                elif func_name.startswith('Array['): # ]
                    func_name = 'Array<' + func_name[6:-1] + '>'
                elif func_name == 'Array': # `list(range(1,10))` -> `Array(1.<10)` -> `create_array(range_el(1, 10))`
                    func_name = 'create_array'
                elif self.children[0].symbol.id == '[' and (self.children[0].is_list or self.children[0].is_dict): # ] # `[Type]()` -> `Array<Type>()`, `[KeyType = ValueType]()` -> `Dict<KeyType, ValueType>()`
                    func_name = trans_type(self.children[0].to_type_str(), self.children[0].scope, self.children[0].token)
                elif func_name == 'Dict':
                    func_name = 'create_dict'
                elif func_name.startswith('DefaultDict['): # ]
                    func_name = 'DefaultDict<' + ', '.join(trans_type(c.to_type_str(), c.scope, c.token) for c in self.children[0].children[1:]) + '>'
                elif func_name in ('Set', 'Deque'):
                    func_name = 'create_set' if func_name == 'Set' else 'create_deque'
                    if self.children[2].is_list:
                        c = self.children[2].children
                        res = func_name + ('<' + trans_type(c[0].children[0].token_str(), self.scope, c[0].children[0].token)
                                         + '>' if len(c) > 1 and c[0].function_call and c[0].children[0].token_str()[0].isupper() else '') + '({'
                        for i in range(len(c)):
                            res += c[i].to_str()
                            if i < len(c)-1:
                                res += ', '
                        return res + '})'
                elif func_name.startswith(('Set[', 'Deque[')): # ]]
                    c = self.children[0].children[1]
                    func_name = func_name[:func_name.find('[')] + '<' + trans_type(c.to_type_str(), c.scope, c.token) + '>' # ]
                elif func_name in ('sum', 'all', 'any', 'min', 'max') and self.children[2].function_call and self.children[2].children[0].symbol.id == '.' and self.children[2].children[0].children[1].token_str() == 'map':
                    assert(len(self.children) == 3)
                    c2 = self.children[2].children[2].to_str()
                    if self.children[2].children[2].token.category == Token.Category.NAME and self.children[2].children[2].token_str()[0].isupper():
                        c2 = '[](const auto &x){return ' + {'Int':'to_int', 'Int64':'to_int64', 'Long':'to_int64', 'UInt64':'to_uint64', 'ULong':'to_uint64', 'UInt32':'to_uint32', 'Float':'to_float', 'SFloat':'to_float32', 'Float32':'to_float32'}.get(c2, c2) + '(x);}'
                    return func_name + '_map(' + self.children[2].children[0].children[0].to_str() + ', ' + c2 + ')'
                elif func_name in ('min', 'max') and len(self.children) == 5 and self.children[3] is not None and self.children[3].token_str() == "key'":
                    return func_name + '_with_key(' + self.children[2].to_str() + ', ' + self.children[4].to_str() + ')'
                elif func_name in ('min', 'max') and len(self.children) == 5 and self.children[3] is not None and self.children[3].token.value(source) == "default'":
                    return func_name + '_with_default(' + self.children[2].to_str() + ', ' + self.children[4].to_str() + ')'
                elif func_name == 'String' and len(self.children) == 5 and self.children[3] is not None and self.children[3].token_str() == "radix'":
                    return 'int_to_str_with_radix(' + self.children[2].to_str() + ', ' + self.children[4].to_str() + ')'
                elif func_name == 'copy':
                    s = self.scope
                    while True:
                        if s.is_function:
                            if s.node is not None and type(s.node.parent) == ASTTypeDefinition:
                                fid = s.parent.ids.get('copy')
                                if fid is not None:
                                    func_name = '::copy'
                            break
                        s = s.parent
                        if s is None:
                            break
                elif func_name == 'move':
                    func_name = 'std::move'
                elif func_name == '*this':
                    func_name = '(*this)' # function call has higher precedence than dereference in C++, so `*this(...)` is equivalent to `*(this(...))`
                elif func_name == 'term::color':
                    if self.children[2].token_str() not in {'GRAY', 'BLUE', 'GREEN', 'CYAN', 'RED', 'MAGENTA', 'YELLOW', 'RESET'}:
                        raise Error('wrong color', self.children[2].left_to_right_token())
                    return func_name + '(term::Color::' + self.children[2].token_str() + ')'
                elif func_name == 'File' and len(self.children) >= 5 and self.children[3] is None:
                    if self.children[4].token_str() not in {'WRITE', 'APPEND'}:
                        raise Error('wrong file open mode', self.children[4].left_to_right_token())
                    args = self.children[2].to_str()
                    if len(self.children) > 5:
                        if len(self.children) != 7 or not (self.children[5] is not None and self.children[5].token_str() == "encoding'"):
                            raise Error('wrong arguments', self.left_to_right_token())
                        args += ', ' + self.children[6].to_str()
                        if self.children[4].token_str() == 'APPEND':
                            args += ', true'
                    else:
                        if self.children[4].token_str() == 'APPEND':
                            args += ', u"utf-8"_S, true'
                    return 'FileWr(' + args + ')'
                elif self.children[0].symbol.id == '[': # ]
                    pass
                elif self.children[0].function_call: # for `enumFromTo(0)(1000)`
                    pass
                else:
                    if self.children[0].symbol.id == ':':
                        fid, sc = find_module(self.children[0].children[0].to_str()).scope.find_and_return_scope(self.children[0].children[1].token_str())
                    elif self.children[0].symbol.id == '.:':
                        fn_name = self.children[0].children[0].token_str()
                        fid = None
                        sc = self.scope
                        while True:
                            if type(sc.node) == ASTTypeDefinition:
                                for child in sc.node.children:
                                    if type(child) == ASTFunctionDefinition and child.is_static and child.function_name == fn_name:
                                        for id_ in sc.ids.values():
                                            if id_.ast_nodes[0] is child:
                                                fid = id_
                                                break
                                        if fid is not None:
                                            break
                                if fid is not None:
                                    break
                            sc = sc.parent
                            if sc is None:
                                break
                    else:
                        fid, sc = self.scope.find_and_return_scope(func_name)
                    if fid is None:
                        raise Error('call of undefined function `' + func_name + '`', self.children[0].left_to_right_token())
                    if len(fid.ast_nodes) > 1:
                        raise Error('functions\' overloading is not supported for now', self.children[0].left_to_right_token())
                    if len(fid.ast_nodes) == 0:
                        if sc.is_function: # for calling of function arguments, e.g. `F amb(comp, ...)...comp(prev, opt)`
                            f_node = None
                        else:
                            raise Error('node of function `' + func_name + '` is not found', self.children[0].left_to_right_token())
                    else:
                        f_node = fid.ast_nodes[0]
                        if type(f_node) == ASTLoop: # for `L(justify) [(s, w) -> ...]...justify(...)`
                            f_node = None
                        else:
                            #assert(type(f_node) in (ASTFunctionDefinition, ASTTypeDefinition) or (type(f_node) in (ASTVariableInitialization, ASTVariableDeclaration) and f_node.function_pointer)
                            #                                                                  or (type(f_node) ==  ASTVariableInitialization and f_node.expression.symbol.id == '->'))
                            if type(f_node) == ASTTypeDefinition:
                                if f_node.has_virtual_functions or f_node.has_pointers_to_the_same_type:
                                    func_name = 'std::make_unique<' + func_name + '>'
                                # elif f_node.has_pointers_to_the_same_type:
                                #     func_name = 'make_SharedPtr<' + func_name + '>'
                                if len(f_node.constructors) == 0:
                                    f_node = ASTFunctionDefinition()
                                else:
                                    if len(f_node.constructors) > 1:
                                        raise Error('constructors\' overloading is not supported for now (see type `' + f_node.type_name + '`)', self.children[0].left_to_right_token())
                                    f_node = f_node.constructors[0]
                            elif type(f_node) == ASTTypeAlias and len(f_node.named_tuple_members) > 0:
                                f_node = f_node.constructor
                last_function_arg = 0
                res = func_name + '('
                for i in range(1, len(self.children), 2):
                    make_ref = self.children[i+1].symbol.id in ('&', 'T &') and len(self.children[i+1].children) == 1 and (self.children[i+1].children[0].is_list
                                                                                                                        or self.children[i+1].children[0].is_dict
                                                                                                                        or self.children[i+1].children[0].function_call)
                    if self.children[i] is None:
                        cstr = self.children[i+1].to_str()
                        if f_node is not None and type(f_node) == ASTFunctionDefinition:
                            if last_function_arg >= len(f_node.function_arguments):
                                raise Error('too many arguments for function `' + func_name + '`', self.children[0].left_to_right_token())
                            if f_node.first_named_only_argument is not None and last_function_arg >= f_node.first_named_only_argument:
                                raise Error('argument `' + f_node.function_arguments[last_function_arg][0] + '` of function `' + func_name + '` is named-only', self.children[i+1].token)
                            if len(f_node.function_arguments[last_function_arg]) > 3 and '&' in f_node.function_arguments[last_function_arg][3] and not (self.children[i+1].symbol.id in ('&', 'T &') and len(self.children[i+1].children) == 1) and self.children[i+1].token_str() not in ('N', 'Н', 'null', 'нуль'):
                                raise Error('argument `' + f_node.function_arguments[last_function_arg][0] + '` of function `' + func_name + '` is in-out, but there is no `&` prefix', self.children[i+1].token)
                            if f_node.function_arguments[last_function_arg][2] == 'File?':
                                tid = self.scope.find(self.children[i+1].token_str())
                                if tid is None or tid.type != 'File?':
                                    res += '&'
                            elif f_node.function_arguments[last_function_arg][2].endswith('?') and f_node.function_arguments[last_function_arg][2].startswith('[') and cstr != 'nullptr': # ] #f_node.function_arguments[last_function_arg][2] != 'Int?' and not cstr.startswith(('std::make_unique<', 'make_SharedPtr<')):
                                res += '&'
                                tid = self.scope.find(cstr)
                                if tid is not None and tid.default_val == 'nullptr':
                                    res = res[:-1] # res -= '&'
                            elif f_node.function_arguments[last_function_arg][2] == 'Char':
                                if self.children[i+1].token.category == Token.Category.STRING_LITERAL:
                                    assert(cstr.startswith('u"') and cstr.endswith('"_S'))
                                    cstr = "u'" + cstr[2:-3] + "'_C"
                        res += 'make_ref('*make_ref + cstr + ')'*make_ref
                        last_function_arg += 1
                    else:
                        if f_node is None or type(f_node) != ASTFunctionDefinition:
                            raise Error('function `' + func_name + '` is not found (you can remove named arguments in function call to suppress this error)', self.children[0].left_to_right_token())
                        argument_name = self.children[i].token_str()[:-1]

                        if argument_name == '':
                            if self.children[i+1].token.category != Token.Category.NAME:
                                raise Error('variable name expected for implicit named argument', self.children[i+1].token)
                            argument_name = self.children[i+1].token_str()

                        while True:
                            if last_function_arg == len(f_node.function_arguments):
                                for arg in f_node.function_arguments:
                                    if arg[0] == argument_name:
                                        raise Error('please correct order of argument `' + argument_name + '`', self.children[i].token)
                                raise Error('argument `' + argument_name + '` is not found in function `' + func_name + '`', self.children[i].token)
                            if f_node.function_arguments[last_function_arg][0] == argument_name:
                                if len(f_node.function_arguments[last_function_arg]) > 3 and '&' in f_node.function_arguments[last_function_arg][3] and not (self.children[i+1].symbol.id in ('&', 'T &') and len(self.children[i+1].children) == 1) and self.children[i+1].token_str() not in ('N', 'Н', 'null', 'нуль'):
                                    raise Error('argument `' + f_node.function_arguments[last_function_arg][0] + '` of function `' + func_name + '` is in-out, but there is no `&` prefix', self.children[i+1].token)
                                last_function_arg += 1
                                break
                            if f_node.function_arguments[last_function_arg][1] == '':
                                raise Error('argument `' + f_node.function_arguments[last_function_arg][0] + '` of function `' + func_name + '` has no default value, please specify its value here', self.children[i].token)
                            res += f_node.function_arguments[last_function_arg][1] + ', '
                            last_function_arg += 1
                        if f_node.function_arguments[last_function_arg-1][2].endswith('?') and not '->' in f_node.function_arguments[last_function_arg-1][2] and self.children[i+1].token_str() not in ('N', 'Н', 'null', 'нуль'):
                            res += '&'
                        res += 'make_ref('*make_ref + self.children[i+1].to_str() + ')'*make_ref
                    if i < len(self.children)-2:
                        res += ', '

                if f_node is not None:
                    if type(f_node) == ASTFunctionDefinition:
                        while last_function_arg < len(f_node.function_arguments):
                            if f_node.function_arguments[last_function_arg][1] == '':
                                t = self.children[len(self.children)-1].rightmost()
                                raise Error('missing required argument `'+ f_node.function_arguments[last_function_arg][0] + '`', Token(t, t, Token.Category.DELIMITER))
                            last_function_arg += 1
                    elif type(f_node) in (ASTTypeEnum, ASTTypeAlias, ASTTypeDefinition):
                        pass
                    elif f_node.function_pointer:
                        if last_function_arg != len(f_node.type_args):
                            raise Error('wrong number of arguments passed to function pointer', Token(self.children[0].token.end, self.children[0].token.end, Token.Category.DELIMITER))

                return res + ')'

            elif self.tuple:
                res = 'make_tuple('
                for i in range(len(self.children)):
                    res += self.children[i].to_str()
                    if i < len(self.children)-1:
                        res += ', '
                return res + ')'

            elif self.is_var_decl:
                assert(self.children[0].symbol.id == '=' and self.children[0].children[0].token.category == Token.Category.NAME)
                return self.children[0].children[0].token_str()

            else:
                assert(len(self.children) == 1)
                if self.children[0].symbol.id in ('..', '.<', '.+', '<.', '<.<'): # чтобы вместо `(range_el(0, seq.len()))` было `range_el(0, seq.len())`
                    return self.children[0].to_str()
                return '(' + self.children[0].to_str() + ')'

        elif self.symbol.id == '[': # ]
            if self.is_list:
                if len(self.children) == 0:
                    raise Error('empty array is not supported', self.left_to_right_token())
                type_of_values_is_char = True
                for child in self.children:
                    if not is_char(child):
                        type_of_values_is_char = False
                        break
                el_type = ''
                if len(self.children) > 1 and self.children[0].function_call and self.children[0].children[0].token_str()[0].isupper() and self.children[0].children[0].token_str() not in ('Array', 'Set'):
                    el_type = trans_type(self.children[0].children[0].token_str(), self.scope, self.children[0].children[0].token)
                res = 'create_array'
                if self.parent is not None and self.parent.symbol.id == '-':
                    res += '_fix_len<' + str(len(self.children))
                    if el_type != '':
                        res += ', ' + el_type
                    res += '>'
                else:
                    if el_type != '':
                        res += '<' + el_type + '>'
                res += '({'
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
                    c0 = child.children[0]
                    if c0.symbol.id == '.' and len(c0.children) == 2 and c0.children[1].token_str().isupper():
                        c0str = c0.to_str().replace('.', '::') # replace `python_to_11l:tokenizer:Token.Category.NAME` with `python_to_11l::tokenizer::Token::Category::NAME`
                    else:
                        c0str = char_or_str(c0, char_key)
                    res += '(' + c0str + ', ' + char_or_str(child.children[1], char_val) + ')'
                return res + ')'
            elif self.children[1].token.category == Token.Category.NUMERIC_LITERAL:
                return '_get<' + self.children[1].to_str() + '>(' + self.children[0].to_str() + ')' # for support tuples (e.g. `(1, 2)[0]` -> `_get<0>(make_tuple(1, 2))`)
            else:
                c1 = self.children[1].to_str()
                if c1.startswith('(len)'):
                    return self.children[0].to_str() + '.at_plus_len(' + c1[len('(len)'):] + ')'
                return self.children[0].to_str() + '[' + c1 + ']'

        elif self.symbol.id == '[%': # ]
            return self.children[0].to_str() + '.at_ni(' + self.children[1].to_str() + ')'

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
                if self.children[i].symbol.id in ('..', '.<', '.+', '<.', '<.<'):
                    res += 'in(a, ' + self.children[i].to_str() + ')'
                elif self.children[i].symbol.id in ('<', '<=', '>', '>='):
                    res += 'a ' + self.children[i].symbol.id + ' ' + self.children[i].children[0].to_str()
                else:
                    res += 'a == ' + (char_or_str(self.children[i], is_char(self.children[i]))[:-2] if self.children[i].token.category == Token.Category.STRING_LITERAL else self.children[i].to_str())
                res += ' ? ' + char_or_str(self.children[i+1], char_val) + ' : '
                # L.was_no_break
                #    res ‘’= ‘throw KeyError(a)’
            return res + ('throw KeyError(a)' if not was_break else '') + ';}(' + self.children[0].to_str() + ')'

        if len(self.children) == 1:
            #return '(' + self.symbol.id + self.children[0].to_str() + ')'
            if self.postfix:
                if self.symbol.id == '!':
                    return '(*' + self.children[0].to_str() + ')'
                elif self.symbol.id == '.*':
                    return '(*' + self.children[0].to_str() + ')'
                return self.children[0].to_str() + self.symbol.id
            elif self.symbol.id == ':':
                c0 = self.children[0].to_str()
                if c0 in ('stdin', 'stdout', 'stderr'):
                    return '_' + c0
                tid = self.scope.find_in_current_function(c0)
                if tid is not None and isinstance(tid.ast_nodes[0], ASTVariableDeclaration) and tid.ast_nodes[0].is_static:
                    return 's_' + c0
                if importing_module:
                    return (importing_module if type(importing_module) == str else os.path.basename(file_name)[:-4]) + '::' + c0
                return '::' + c0
            elif self.symbol.id == '.':
                c0 = self.children[0].to_str()
                sn = self
                while True:
                    if sn.symbol.id == '.' and len(sn.children) == 3:
                        return 'T.' + c0 + '()'*(c0 in ('len', 'last', 'empty')) # T means *‘t’emporary [variable], and it can be safely used because `T` is a keyletter
                    if sn.parent is None:
                        n = sn.ast_parent
                        while n is not None:
                            if type(n) == ASTWith:
                                return 'T.' + c0
                            n = n.parent
                        break
                    sn = sn.parent
                if self.scope.find_in_current_function(c0):
                    return 'this->' + c0
                else:
                    return c0
            elif self.symbol.id == '.:':
                return 's_' + self.children[0].to_str()
            elif self.symbol.id == '..':
                c0 = self.children[0].to_str()
                if c0.startswith('(len)'):
                    return 'range_elen_i(' + c0[len('(len)'):] + ')'
                else:
                    return 'range_ei(' + c0 + ')'
            elif self.symbol.id in ('&', 'T &'):
                if not self.parent.function_call and not (self.parent.symbol.id == '>>' and self is self.parent.children[1]):
                    raise Error('wrong usage of unary `&` operator', self.token)
                return self.children[0].to_str()
            elif self.symbol.id == '-' and self.children[0].is_list:
                return self.children[0].to_str()
            else:
                return self.symbol.id + self.children[0].to_str()
        elif len(self.children) == 2:
            #return '(' + self.children[0].to_str() + ' ' + self.symbol.id + ' ' + self.children[1].to_str() + ')'

            def char_if_len_1(child):
                return char_or_str(child, is_char(child))

            if self.symbol.id == '.':
                cts0 = self.children[0].token_str()
                c1 = self.children[1].to_str()
                if cts0.endswith('@'):
                    assert(cts0 == '@' * len(cts0))
                    if self.scope.find_in_current_type_function(c1):
                        return 'this->' + c1
                    else:
                        return c1
                if cts0 == '.' and len(self.children[0].children) == 1: # `.left.tree_indent()` -> `left->tree_indent()`
                    c00 = self.children[0].children[0].token_str()
                    scp = self.scope
                    while not (type(scp.node) == ASTFunctionDefinition and type(scp.node.parent) == ASTTypeDefinition):
                        scp = scp.parent
                    id_ = scp.node.parent.find_id_including_base_types(c00)
                    if id_ is not None and len(id_.ast_nodes) and type(id_.ast_nodes[0]) in (ASTVariableInitialization, ASTVariableDeclaration):
                        if self.scope.find_in_current_function(c00):
                            c00 = 'this->' + c00
                        if id_.ast_nodes[0].is_reference:
                            return c00 + '->' + c1
                        tid = self.scope.find(id_.ast_nodes[0].type.rstrip('?'))
                        if tid is not None and len(tid.ast_nodes) and type(tid.ast_nodes[0]) == ASTTypeDefinition and (tid.ast_nodes[0].has_pointers_to_the_same_type or tid.ast_nodes[0].has_virtual_functions):
                            return c00 + '->' + c1

                if cts0 == ':' and len(self.children[0].children) == 1: # `:token_node.symbol` -> `::token_node->symbol`
                    id_ = global_scope.find(self.children[0].children[0].token_str())
                    if id_ is not None and len(id_.ast_nodes) and type(id_.ast_nodes[0]) in (ASTVariableInitialization, ASTVariableDeclaration):
                        tid = self.scope.find(id_.ast_nodes[0].type)#.rstrip('?')
                        if tid is not None and len(tid.ast_nodes) and type(tid.ast_nodes[0]) == ASTTypeDefinition and tid.ast_nodes[0].has_pointers_to_the_same_type:
                            return '::' + self.children[0].children[0].token_str() + '->' + c1

                if cts0 == '.' and len(self.children[0].children) == 2: # // for `ASTNode token_node; token_node.symbol.id = sid` -> `... token_node->symbol->id = sid`
                    t_node = type_of(self.children[0])                  # \\ and `ASTNode token_node; ... :token_node.symbol.id = sid` -> `... ::token_node->symbol->id = sid`
                    if t_node is not None and type(t_node) in (ASTVariableDeclaration, ASTVariableInitialization) and (t_node.is_reference or t_node.is_ptr): # ( # t_node.is_shared_ptr):
                        return self.children[0].to_str() + '->' + c1
                    if t_node is not None and type(t_node) == ASTTypeDefinition:
                        for child in t_node.children:
                            if type(child) == ASTFunctionDefinition and child.is_static and child.function_name == c1:
                                return self.children[0].to_str() + '::s_' + c1
                        return self.children[0].to_str() + '::' + c1

                if cts0 == '(': # ) # `parse(expr_str).eval()` -> `parse(expr_str)->eval()`
                    fid, sc = self.scope.find_and_return_scope(self.children[0].children[0].token_str())
                    if fid is not None and len(fid.ast_nodes) == 1:
                        f_node = fid.ast_nodes[0]
                        if type(f_node) == ASTFunctionDefinition and f_node.function_return_type != '':
                            frtid = sc.find(f_node.function_return_type)
                            if frtid is not None and len(frtid.ast_nodes) == 1 and type(frtid.ast_nodes[0]) == ASTTypeDefinition and frtid.ast_nodes[0].has_pointers_to_the_same_type:
                                return self.children[0].to_str() + '->' + c1

                if cts0 in ('Float', 'SFloat', 'Float32', 'Float64') and c1 == 'infinity':
                    return 'std::numeric_limits<' + cpp_type_from_11l[cts0] + '>::infinity()'

                id_, s = self.scope.find_and_return_scope(cts0.lstrip('@='))
                if id_ is not None:
                    if id_.type != '' and id_.type.endswith('?'):
                        return cts0.lstrip('@=') + '->' + c1 + '()'*(c1 in ('len', 'last', 'empty'))
                    if len(id_.ast_nodes) and type(id_.ast_nodes[0]) == ASTLoop and id_.ast_nodes[0].is_loop_variable_a_ptr and cts0 == id_.ast_nodes[0].loop_variable:
                        return cts0 + '->' + c1
                    if len(id_.ast_nodes) and type(id_.ast_nodes[0]) == ASTVariableInitialization and (id_.ast_nodes[0].is_ptr): # ( # or id_.ast_nodes[0].is_shared_ptr):
                        return self.children[0].to_str() + '->' + c1 + '()'*(c1 in ('len', 'last', 'empty')) # `to_str()` is needed for such case: `animal.say(); animals [+]= animal; animal.say()` -> `animal->say(); animals.append(animal); std::move(animal)->say();`
                    if len(id_.ast_nodes) and type(id_.ast_nodes[0]) in (ASTVariableInitialization, ASTVariableDeclaration): # `Node tree = ...; tree.tree_indent()` -> `... tree->tree_indent()` # (
                        tid = self.scope.find(id_.ast_nodes[0].type)#.rstrip('?'))
                        if tid is not None and len(tid.ast_nodes) and type(tid.ast_nodes[0]) == ASTTypeDefinition and tid.ast_nodes[0].has_pointers_to_the_same_type:
                            return cts0 + '->' + c1
                    if len(id_.ast_nodes) and type(id_.ast_nodes[0]) == ASTTypeDefinition:
                        for child in id_.ast_nodes[0].children:
                            if type(child) == ASTFunctionDefinition and child.is_static and child.function_name == c1:
                                return cts0 + '::s_' + c1
                            if type(child) == ASTTypeDefinition and child.type_name == c1:
                                return cts0 + '::' + c1
                    if id_.type != '' and s.is_function:
                        tid = s.find(id_.type)
                        if tid is not None and len(tid.ast_nodes) and type(tid.ast_nodes[0]) == ASTTypeDefinition and tid.ast_nodes[0].has_pointers_to_the_same_type:
                            return cts0 + '->' + c1

                if c1.isupper() and not (self.parent is not None and self.parent.function_call and self is self.parent.children[0]):
                    c0 = self.children[0].to_str()
                    #assert(c0[0].isupper())
                    return c0.replace('.', '::') + '::' + c1 # replace `Token.Category.STATEMENT_SEPARATOR` with `Token::Category::STATEMENT_SEPARATOR`

                return char_if_len_1(self.children[0]) + '.' + c1 + '()'*(c1 in ('len', 'last', 'empty', 'real', 'imag') and not (self.parent is not None and self.parent.function_call and self is self.parent.children[0])) # char_if_len_1 is needed here because `u"0"_S.code` (have gotten from #(11l)‘‘0’.code’) is illegal [correct: `u'0'_C.code`]

            elif self.symbol.id == ':':
                c0 = self.children[0].to_str()
                c0 = {'time':'timens', # 'time': a symbol with this name already exists and therefore this name cannot be used as a namespace name
                      'random':'randomns', # GCC: .../11l-lang/_11l_to_cpp/11l_hpp/random.hpp:1:11: error: ‘namespace random { }’ redeclared as different kind of symbol
                      'c':'c_ns', # `c` is a too short name for namespace (there may be conflicts with variable naming)
                     }.get(c0, c0)
                c1 = self.children[1].to_str()
                return c0 + '::' + (c1 if c1 != '' else '_')

            elif self.symbol.id == '.:':
                return self.children[0].to_str() + '::s_' + self.children[1].to_str()

            elif self.symbol.id == '->':
                captured_variables = set()
                def gather_captured_variables(sn, capture_level = 0):
                    if sn.token.category == Token.Category.NAME:
                        if sn.token_str().startswith('@'):
                            by_ref = True # sn.parent.children[0] is sn and ((sn.parent.symbol.id[-1] == '=' and sn.parent.symbol.id not in ('==', '!='))
                                          #                               or (sn.parent.symbol.id == '.' and sn.parent.children[1].token_str() == 'append'))
                            c = sn.token_str().count('@')
                            if c > capture_level:
                                t = sn.token_str()[c:]
                                if t.startswith('='):
                                    t = t[1:]
                                    by_ref = False
                                if not t.startswith(':'):
                                    captured_variables.add('this' if t == '' else '&'*by_ref + t)
                        elif sn.token.value(source) == '(.)':
                            captured_variables.add('this')
                    else:
                        for child in sn.children:
                            if child is not None:
                                gather_captured_variables(child, capture_level + int(child.symbol is not None and child.symbol.id == '->'))
                gather_captured_variables(self.children[1])
                return '[' + ', '.join(sorted(captured_variables)) + '](' + ', '.join(map(lambda c: 'const ' + ('auto &' if c.symbol.id != '=' else 'decltype(' + c.children[1].to_str() + ') &') + c.to_str(),
                    self.children[0].children if self.children[0].symbol.id == '(' else [self.children[0]])) + '){return ' + self.children[1].to_str() + ';}' # )
            elif self.symbol.id in ('..', '.<', '.+', '<.', '<.<'):
                s = {'..':'ee', '.<':'el', '.+':'ep', '<.':'le', '<.<':'ll'}[self.symbol.id]
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
            elif self.symbol.id in ('C', 'С', 'in', 'св'):
                return 'in(' + char_if_len_1(self.children[0]) + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('!C', '!С', '!in', '!св'):
                return '!in(' + char_if_len_1(self.children[0]) + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('I/', 'Ц/'):
                return 'idiv(' + self.children[0].to_str() + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('-I/', '-Ц/'):
                return 'nidiv(' + self.children[0].to_str() + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('I/=', 'Ц/='):
                return self.children[0].to_str() + ' = idiv(' + self.children[0].to_str() + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id in ('==', '!=', '=') and self.children[1].token.category == Token.Category.NAME and self.children[1].token_str().isupper() and self.scope.find(self.children[1].token_str()) is None: # `token.category == NAME` -> `token.category == TYPE_RM_REF(token.category)::NAME` and `category = NAME` -> `category = TYPE_RM_REF(category)::NAME`
                return self.children[0].to_str() + ' ' + self.symbol.id + ' TYPE_RM_REF(' + self.children[0].to_str() + ')::' + self.children[1].token_str()
            elif self.symbol.id in ('==', '!=') and self.children[0].symbol.id in ('&', 'T &') and len(self.children[0].children) == 1 and self.children[1].symbol.id in ('&', 'T &') and len(self.children[1].children) == 1: # `&a == &b` -> `&a == &b`
                id_, s = self.scope.find_and_return_scope(self.children[0].children[0].token_str())
                if id_ is not None and len(id_.ast_nodes) and type(id_.ast_nodes[0]) == ASTLoop and id_.ast_nodes[0].is_loop_variable_a_ptr and self.children[0].children[0].token_str() == id_.ast_nodes[0].loop_variable: # `L(obj)...&obj != &objChoque` -> `...&*obj != objChoque`
                    return '&*' + self.children[0].children[0].token_str() + ' ' + self.symbol.id + ' ' + self.children[1].children[0].token_str()
                return '&' + self.children[0].children[0].token_str() + ' ' + self.symbol.id + ' &' + self.children[1].children[0].to_str()
            elif self.symbol.id == '==' and self.children[0].symbol.id == '==': # replace `a == b == c` with `equal(a, b, c)`
                def f(child):
                    if child.symbol.id == '==':
                        return f(child.children[0]) + ', ' + char_if_len_1(child.children[1])
                    return child.to_str()
                return 'equal(' + f(self) + ')'
            elif self.symbol.id in ('==', '!=', '<', '<=', '>', '>=') and self.children[1].token.category == Token.Category.STRING_LITERAL:
                return self.children[0].to_str() + ' ' + self.symbol.id + ' ' + char_if_len_1(self.children[1])[:-2]
            elif self.symbol.id == '=' and self.children[0].symbol.id == '[': # ] # replace `a[k] = v` with `a.set(k, v)`
                if self.children[0].children[1].token.category == Token.Category.NUMERIC_LITERAL: # replace `a[0] = v` with `_set<0>(a, v)` to support tuples
                    return '_set<' + self.children[0].children[1].to_str() + '>(' + self.children[0].children[0].to_str() + ', ' + char_if_len_1(self.children[1]) + ')'
                else:
                    c01 = self.children[0].children[1].to_str()
                    if c01.startswith('(len)'):
                        return self.children[0].children[0].to_str() + '.set_plus_len(' + c01[len('(len)'):] + ', ' + char_if_len_1(self.children[1]) + ')'
                    else:
                        return self.children[0].children[0].to_str() + '.set(' + c01 + ', ' + char_if_len_1(self.children[1]) + ')'
            elif self.symbol.id == '[+]=': # replace `a [+]= v` with `a.append(v)`
                return self.children[0].to_str() + '.append(' + self.children[1].to_str() + ')'
            elif self.symbol.id == '=' and self.children[0].tuple:
                #assert(False)
                return 'assign_from_tuple(' + ', '.join(c.to_str() for c in self.children[0].children) + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id == '?':
                parens = self.parent is not None and self.parent.symbol.id == '[' # ] # to fix MSVC error C3750
                return '('*parens + '[&]{auto R = ' + self.children[0].to_str() + '; return R != nullptr ? *R : ' + self.children[1].to_str() + ';}()' + ')'*parens
            elif self.symbol.id == '^':
                c1 = self.children[1].to_str()
                if c1 in ('2', '2LL'):
                    return 'square(' + self.children[0].to_str() + ')'
                if c1 in ('3', '3LL'):
                    return 'cube(' + self.children[0].to_str() + ')'
                return 'pow(' + self.children[0].to_str() + ', ' + c1 + ')'
            elif self.symbol.id == '%':
                return 'mod(' + self.children[0].to_str() + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id == '-%':
                return 'nmod(' + self.children[0].to_str() + ', ' + self.children[1].to_str() + ')'
            elif self.symbol.id == '[&]' and self.parent is not None and self.parent.symbol.id in ('==', '!=', '<', '<=', '>', '>='): # there is a difference in precedence of operators `&` and `==`/`!=` in Python/11l and C++
                return '(' + self.children[0].to_str() + ' & ' + self.children[1].to_str() + ')'
            elif self.symbol.id == '(+)' and self.parent is not None and self.parent.symbol.id in ('==', '!=', '<', '<=', '>', '>='): # there is a difference in precedence of operators `^` and `<`/`<=`/`>`/`>=` in Python/11l and C++
                return '(' + self.children[0].to_str() + ' ^ ' + self.children[1].to_str() + ')'
            elif self.symbol.id == '(concat)' and self.parent is not None and self.parent.symbol.id in ('+', '-', '==', '!='): # `print(‘id = ’id+1)` -> `print((‘id = ’id)+1)`, `a & b != u"1x"` -> `(a & b) != u"1x"` [[[`'-'` is needed because `print(‘id = ’id-1)` also should generate a compile-time error]]]
                return '(' + self.children[0].to_str() + ' & ' + self.children[1].to_str() + ')'
            elif self.symbol.id == '*' and self.children[0].symbol.id == '-' and len(self.children[0].children) == 1 and self.children[0].children[0].is_list:
                assert(len(self.children[0].children[0].children) == 1)
                return 'create_array_fix_len_el<' + self.children[1].to_str() + '>(' + self.children[0].children[0].children[0].to_str() + ')'
            else:
                def is_integer(t):
                    return t.category == Token.Category.NUMERIC_LITERAL and ('.' not in t.value(source)) and ('e' not in t.value(source))
                if self.symbol.id == '/' and (is_integer(self.children[0].token) or is_integer(self.children[1].token)):
                    if is_integer(self.children[0].token):
                        return self.children[0].token_str() + '.0 / ' + self.children[1].to_str()
                    else:
                        return self.children[0].to_str() + ' / ' + self.children[1].token_str() + '.0'

                if self.symbol.id == '/' and python_division:
                    return 'fdiv(' + self.children[0].to_str() + ', ' + self.children[1].to_str() + ')'

                if self.symbol.id == '=' and self.children[0].symbol.id == '.' and len(self.children[0].children) == 2: # `:token_node.symbol = :symbol_table[...]` -> `::token_node->symbol = &::symbol_table[...]`
                    t_node = type_of(self.children[0])
                    if t_node is not None and type(t_node) in (ASTVariableDeclaration, ASTVariableInitialization) and t_node.is_reference:
                        c1s = self.children[1].to_str()
                        return self.children[0].to_str() + ' = ' + '&'*(c1s != 'nullptr') + c1s

                if self.symbol.id == '=':
                    return self.children[0].to_str() + ' = ' + char_if_len_1(self.children[1])

                return self.children[0].to_str() + ' ' + {'&':'&&', '|':'||', '[&]':'&', '[&]=':'&=', '[|]':'|', '[|]=':'|=', '(concat)':'&', '[+]':'+', '‘’=':'&=', '(+)':'^', '(+)=':'^='}.get(self.symbol.id, self.symbol.id) + ' ' + self.children[1].to_str()
        elif len(self.children) == 3:
            if self.children[1].token.category == Token.Category.SCOPE_BEGIN:
                assert(self.symbol.id == '.')
                if self.children[2].symbol.id == '?': # not necessary, just to beautify generated C++
                    return '[&](auto &&T){auto X = ' + self.children[2].children[0].to_str() + '; return X != nullptr ? *X : ' + self.children[2].children[1].to_str() + ';}(' + self.children[0].to_str() + ')'
                return '[&](auto &&T){return ' + self.children[2].to_str() + ';}(' + self.children[0].to_str() + ')' # why I prefer `auto &&T` to `auto&& T`: ampersand is related to the variable, but not to the type, for example in `int &i, j` `j` is not a reference, but just an integer
            assert(self.symbol.id in ('I', 'Е', 'if', 'если'))
            result_is_char = is_char(self.children[1]) and is_char(self.children[2])
            return self.children[0].to_str() + ' ? ' + char_or_str(self.children[1], result_is_char) + ' : ' + char_or_str(self.children[2], result_is_char)

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
        if id[0].isalpha() and not id in ('I/', 'Ц/', 'I/=', 'Ц/=', 'C', 'С', 'in', 'св', 'T? ', 'T :', 'T &', 'T ='): # this is keyword-in-expression
            assert(id.isalpha() or id in ('L.last_iteration', 'Ц.последняя_итерация', 'loop.last_iteration', 'цикл.последняя_итерация'))
            allowed_keywords_in_expressions.append(id)
    else:
        s.lbp = max(bp, s.lbp)
    return s

class ASTNode:
    parent : 'ASTNode' = None
    access_specifier_public = 1

    def walk_expressions(self, f):
        pass
    def walk_children(self, f):
        pass

def pre_nl(toki = None):
    if toki is None:
        toki = tokeni
    if toki > 0:
        ti = toki - 1
        while ti > 0 and (tokens[ti].category in (Token.Category.SCOPE_END, Token.Category.STATEMENT_SEPARATOR) or (tokens[ti].category == Token.Category.SCOPE_BEGIN and tokens[ti].value(source) != '{')): # }
            ti -= 1
        lines_with_comments = len(re.findall(r'\n[ \t]*//',
                    source[tokens[ti].end:tokens[toki].start]))
        return (min(source[tokens[ti].end:tokens[toki].start].count("\n") - lines_with_comments, 2) - 1) * "\n"
    return ''

class ASTNodeWithChildren(ASTNode):
    # children : List['ASTNode'] = [] # OMFG! This actually means static (common for all objects of type ASTNode) variable, not default value of member variable, that was unexpected to me as it contradicts C++11 behavior
    children : List['ASTNode']
    tokeni : int
    #scope : Scope

    def __init__(self):
        self.children = []
        self.tokeni = tokeni

    def walk_children(self, f):
        for child in self.children:
            f(child)

    def children_to_str(self, indent, t, place_opening_curly_bracket_on_its_own_line = True, add_at_beginning = ''):
        r = pre_nl(self.tokeni)
        r += ' ' * (indent*4) + t + (("\n" + ' ' * (indent*4) + "{\n") if place_opening_curly_bracket_on_its_own_line else " {\n") # }
        r += add_at_beginning
        for c in self.children:
            s = c.to_str(indent+1)
            if c is self.children[0] and add_at_beginning.endswith("\n\n"):
                s = s.lstrip("\n")
            r += s
        return r + ' ' * (indent*4) + "}\n"

    def children_to_str_detect_single_stmt(self, indent, r, check_for_if = False):
        def has_if(node):
            while True:
                if not isinstance(node, ASTNodeWithChildren):
                    return False
                if type(node) == ASTIf:
                    return True
                if len(node.children) != 1:
                    return False
                node = node.children[0]
        if (len(self.children) != 1
                or (check_for_if and (type(self.children[0]) == ASTIf or (has_if(self.children[0]) and type(self) in (ASTIf, ASTElseIf) and self.else_or_elif is not None))) # for correctly handling of dangling-else
                or type(self.children[0]) == ASTLoopRemoveCurrentElementAndContinue # `L.remove_current_element_and_continue` ‘раскрывается в 2 строки кода’\‘is translated into 2 statements’
                or (type(self.children[0]) == ASTTupleAssignment and self.children[0].is_multi_st()) # for `mx, mx_index = digit, i` in [https://www.rosettacode.org/wiki/Next_highest_int_from_digits#Python:_Algorithm_2]
                or (type(self.children[0]) == ASTLoop and (self.children[0].has_L_was_no_break() or self.children[0].break_label_needed != -1))
                or (type(self.children[0]) == ASTSwitch and self.children[0].has_string_case)):
            return self.children_to_str(indent, r, False)
        assert(len(self.children) == 1)
        return pre_nl(self.tokeni) + ' ' * (indent*4) + r + "\n" + self.children[0].to_str(indent+1)

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
        prev_global_statement = True
        code_block_id = 1

        for c in self.children:
            global_statement = type(c) in (ASTVariableDeclaration, ASTVariableInitialization, ASTTupleInitialization, ASTFunctionDefinition, ASTTypeDefinition, ASTTypeAlias, ASTTypeEnum, ASTMain, ASTIncludeFile)
            beginning_of_codeblock = False
            if global_statement != prev_global_statement:
                beginning_of_codeblock = True
                prev_global_statement = global_statement
                if not global_statement:
                    sname = 'CodeBlock' + str(code_block_id)
                    r += "\n"*(c is not self.children[0]) + 'struct ' + sname + "\n{\n    " + sname + "()\n    {\n"
                else:
                    r += "    }\n} code_block_" + str(code_block_id) + ";\n\n"
                    code_block_id += 1
            s = c.to_str(2*(not global_statement))
            if beginning_of_codeblock:
                s = s.lstrip("\n")
            r += s

        if prev_global_statement != True: # {{
            r += "    }\n} code_block_" + str(code_block_id) + ";\n"

        return r

class ASTExpression(ASTNodeWithExpression):
    def to_str(self, indent):
        if self.expression.symbol.id == '=' and type(self.parent) == ASTTypeDefinition:
            if self.expression.children[0].symbol.id == '-' and len(self.expression.children[0].children) == 1:
                if self.expression.children[0].children[0].symbol.id == ':' and len(self.expression.children[0].children[0].children) == 1:
                    return self.pre_nl + ' ' * (indent*4) + 'static inline const auto s_' + self.expression.children[0].children[0].children[0].to_str() + ' = ' + self.expression.children[1].to_str() + ";\n"
                return self.pre_nl + ' ' * (indent*4) + 'const decltype(' + self.expression.children[1].to_str() + ') ' + self.expression.children[0].children[0].to_str() + ' = ' + self.expression.children[1].to_str() + ";\n"
            if self.expression.children[0].symbol.id == ':' and len(self.expression.children[0].children) == 1:
                return self.pre_nl + ' ' * (indent*4) + 'static inline auto s_' + self.expression.children[0].children[0].to_str() + ' = ' + self.expression.children[1].to_str() + ";\n"
            return self.pre_nl + ' ' * (indent*4) + 'decltype(' + self.expression.children[1].to_str() + ') ' + self.expression.to_str() + ";\n"
        return self.pre_nl + ' ' * (indent*4) + self.expression.to_str() + ";\n"

cpp_type_from_11l = {'auto&':'auto&', 'V':'auto', 'П':'auto', 'var':'auto', 'пер':'auto',
                     'Int':'int', 'Long':'Long', 'ULong':'ULong', 'Int64':'Int64', 'UInt64':'UInt64', 'Int32':'int32_t', 'UInt32':'uint32_t', 'Int16':'int16_t', 'UInt16':'uint16_t', 'Int8':'int8_t', 'BigInt':'BigInt', 'Size':'Size', 'USize':'USize',
                     'Float':'double', 'SFloat':'float', 'Float32':'float', 'Float64':'double', 'Complex':'Complex', 'String':'String', 'Bool':'bool', 'Byte':'Byte', 'Bytes':'Array<Byte>', 'Ptr':'Ptr', 'Void':'void',
                     'Array':'Array', 'ArrayFixLen':'ArrayFixLen', 'ArrayMaxLen':'ArrayMaxLen', 'Tuple':'Tuple', 'Dict':'Dict', 'DefaultDict':'DefaultDict', 'Set':'Set', 'Deque':'Deque', 'Counter':'Counter', 'Fraction':'Fraction'}
cpp_vectype_from_11l = {}
for dimension in ('2', '3', '4'):
    cpp_vectype_from_11l['BVec' + dimension] ='ubvec' + dimension
    cpp_vectype_from_11l['I8Vec'+ dimension] ='sbvec' + dimension
    cpp_vectype_from_11l['IVec' + dimension] = 'ivec' + dimension
    cpp_vectype_from_11l['SVec' + dimension] =  'vec' + dimension
    cpp_vectype_from_11l['DVec' + dimension] = 'dvec' + dimension
    cpp_vectype_from_11l['I16Vec' + dimension] =  'svec' + dimension
    cpp_vectype_from_11l['U16Vec' + dimension] = 'usvec' + dimension
    cpp_vectype_from_11l['I64Vec' + dimension] = 'llvec' + dimension
    cpp_vectype_from_11l['LVec'   + dimension] = 'llvec' + dimension
    cpp_vectype_from_11l['BoolVec' + dimension] = 'bvec' + dimension
for _11l_vectype, cpp_vectype in cpp_vectype_from_11l.items():
    cpp_type_from_11l[_11l_vectype] = cpp_vectype

def trans_type(ty, scope, type_token, ast_type_node = None, is_reference = False):
    if ty.startswith('__EXPRESSION__('):
        assert(ty[-1] == ')')
        return ty[15:-1]

    if ty[-1] == '?':
        if ty not in ('V?', 'П?', 'var?', 'пер?'):
            tt = trans_type(ty[:-1], scope, type_token, ast_type_node, is_reference)
            if not tt.startswith('std::unique_ptr<'):
                return 'Nullable<' + tt + '>'
        ty = ty[:-1]

    t = cpp_type_from_11l.get(ty)
    if t is not None:
        if t == 'int' and int_is_int64:
            return 'Int64'
        return t
    else:
        if '.' in ty: # for `Token.Category category`
            return ty.replace('.', '::') # [-TODO: generalize-]
        if ':' in ty: # for `[symasm:Error] errors`
            return ty.replace(':', '::') # [-TODO: generalize-]

        if ty.startswith('TYPE_OF('):
            assert(ty[-1] == ')')
            return ty

        if ty.startswith('('):
            assert(ty[-1] == ')')
            i = 1
            s = i
            nesting_level = 0
            types = ''
            while True:
                if ty[i] in ('(', '['):
                    nesting_level += 1
                elif ty[i] in (')', ']'):
                    if nesting_level == 0:
                        assert(i == len(ty)-1)
                        types += trans_type(ty[s:i], scope, type_token, ast_type_node)
                        break
                    nesting_level -= 1
                elif ty[i] == ',':
                    if nesting_level == 0: # ignore inner commas
                        types += trans_type(ty[s:i], scope, type_token, ast_type_node) + ', '
                        i += 1
                        while ty[i] == ' ':
                            i += 1
                        s = i
                        continue
                i += 1
            tuple_types = types.split(', ')
            if tuple_types[0] in ('int', 'Int64', 'float', 'double') and tuple_types.count(tuple_types[0]) == len(tuple_types) and len(tuple_types) in range(2, 5):
                return {'int':'i', 'Int64':'ll', 'float':'', 'double':'d'}[tuple_types[0]] + 'vec' + str(len(tuple_types))
            return 'Tuple<' + types + '>'

        p = ty.find('[') # ]
        if p != -1:
            if '=' in ty:
                assert(p == 0 and ty[0] == '[' and ty[-1] == ']')
                tylist = ty[1:-1].split('=')
                assert(len(tylist) == 2)
                return 'Dict<' + trans_type(tylist[0], scope, type_token, ast_type_node) + ', ' \
                               + trans_type(tylist[1], scope, type_token, ast_type_node) + '>'
            if ty.startswith('Callable['): # ]
                tylist = ty[p+1:-1].split(', ')
                def trans_ty(ty):
                    tt = trans_type(ty, scope, type_token, ast_type_node)
                    return tt if tt.startswith('std::unique_ptr<') else 'const ' + tt + ('&'*(ty not in ('Int', 'Float')))
                return 'std::function<' + trans_type(tylist[-1], scope, type_token, ast_type_node) + '(' + ', '.join(trans_ty(t) for t in tylist[:-1]) + ')>'
            if ty.startswith('Tuple['): # ]
                tuple_types = ty[6:-1].split(', ')
                if tuple_types[0] in ('Int', 'Int64', 'Float32', 'SFloat', 'Float') and tuple_types.count(tuple_types[0]) == len(tuple_types) and len(tuple_types) in range(2, 5):
                    return {'Int':'i', 'Int64':'ll', 'Float32':'', 'SFloat':'', 'Float':'d'}[tuple_types[0]] + 'vec' + str(len(tuple_types))
            return (trans_type(ty[:p], scope, type_token, ast_type_node) if p != 0 else 'Array') + '<' + trans_type(ty[p+1:-1], scope, type_token, ast_type_node) + '>'
        p = ty.find(',')
        if p != -1:
            return trans_type(ty[:p], scope, type_token, ast_type_node) + ', ' + trans_type(ty[p+1:].lstrip(' '), scope, type_token, ast_type_node)

        id = scope.find(ty)
        if id is None or len(id.ast_nodes) == 0:
            raise Error('type `' + ty + '` is not defined', type_token)
        if type(id.ast_nodes[0]) in (ASTTypeAlias, ASTTypeEnum):
            return ty
        if type(id.ast_nodes[0]) != ASTTypeDefinition:
            raise Error('`' + ty + '`: expected a type name', type_token)
        if id.ast_nodes[0].has_virtual_functions or id.ast_nodes[0].has_pointers_to_the_same_type:
            if ast_type_node is not None and tokens[id.ast_nodes[0].tokeni].start > type_token.start: # if type `ty` was declared after this variable, insert a forward declaration of type `ty`
                ast_type_node.forward_declared_types.add(ty)
            return ty if is_reference else 'std::unique_ptr<' + ty + '>'# if id.ast_nodes[0].has_virtual_functions else 'SharedPtr<' + ty + '>'
        return ty

class ASTVariableDeclaration(ASTNode):
    vars : List[str]
    type : str
    type_args : List[str]
    is_const = False
    is_static = False
    function_pointer = False
    is_reference = False
    scope : Scope
    type_token : Token
    is_ptr = False
    nullable = False
    #is_shared_ptr = False

    def __init__(self):
        self.scope = scope

    def trans_type(self, ty, is_reference = False):
        if ty.endswith('&'):
            assert(trans_type(ty[:-1], self.scope, self.type_token, self.parent if type(self.parent) == ASTTypeDefinition else None, is_reference) == 'auto')
            return 'auto&'
        return trans_type(ty, self.scope, self.type_token, self.parent if type(self.parent) == ASTTypeDefinition else None, is_reference)

    def to_str(self, indent):
        if self.function_pointer:
            def trans_type(ty):
                tt = self.trans_type(ty)
                return tt if tt.startswith('std::unique_ptr<') else 'const ' + tt + ('&'*(ty not in ('Int', 'Float')))
            return self.pre_nl + ' ' * (indent*4) + 'std::function<' + self.trans_type(self.type) + '(' + ', '.join(trans_type(ty) for ty in self.type_args) + ')> ' + ', '.join(self.vars) + ";\n"
        if self.type.endswith('?') and self.type not in ('V?', 'П?', 'var?', 'пер?'):
            tt = self.trans_type(self.type)
            if not tt.startswith(('std::unique_ptr<', 'Nullable<')):
                assert(not self.is_const and not self.is_reference)
                return ' ' * (indent*4) + 'Nullable<' + tt + ('<' + ', '.join(self.trans_type(ty) for ty in self.type_args) + '>' if len(self.type_args) else '') + '> ' + ', '.join(self.vars) + ";\n"
        static_decl = ''
        if self.is_static:
            static_decl = 'static inline ' if type(self.parent) == ASTTypeDefinition else 'static '
        if self.type == 'Array' and self.type_args == ['Bool']:
            type_name = 'Array<char>'
        elif self.type == 'File' and self.type_args == ['WRITE']:
            type_name = 'FileWr'
        else:
            type_name = self.trans_type(self.type, self.is_reference) + ('<' + ', '.join(self.trans_type(ty) for ty in self.type_args) + '>' if len(self.type_args) else '')
        return self.pre_nl + ' ' * (indent*4) + 'const '*self.is_const + static_decl + type_name \
                           + ' ' + '*'*self.is_reference + 's_'*self.is_static + ', '.join(self.vars) + ";\n"

class ASTVariableInitialization(ASTVariableDeclaration, ASTNodeWithExpression):
    def to_str(self, indent):
        return super().to_str(indent)[:-2] + ' = ' + self.expression.to_str() + ";\n"

class ASTTupleInitialization(ASTNodeWithExpression):
    dest_vars : List[str]
    is_const = False
    bind_array = False

    def __init__(self):
        self.dest_vars = []
        self.pre_nl = pre_nl()

    def to_str(self, indent):
        e = self.expression.to_str()
        if self.bind_array:
            e = 'bind_array<' + str(len(self.dest_vars)) + '>(' + e + ')'
        return self.pre_nl + ' ' * (indent*4) + 'const '*self.is_const + 'auto [' + ', '.join(self.dest_vars) + '] = ' + e + ";\n"

class ASTTupleAssignment(ASTNodeWithExpression):
    dest_vars : List[Tuple[str, bool]]

    def __init__(self):
        self.dest_vars = []
        self.pre_nl = pre_nl()

    def is_multi_st(self):
        return any(b for s, b in self.dest_vars)

    def to_str(self, indent):
        r = self.pre_nl
        for i, dv in enumerate(self.dest_vars):
            if dv[1]:
                r += ' ' * (indent*4) + 'TUPLE_ELEMENT_T(' + str(i) + ', ' + self.expression.to_str() + ') ' + dv[0] + ";\n"
        return r + ' ' * (indent*4) + 'assign_from_tuple(' + ', '.join(dv[0] for dv in self.dest_vars) + ', ' + self.expression.to_str() + ')' + ";\n"

class ASTWith(ASTNodeWithChildren, ASTNodeWithExpression):
    def to_str(self, indent):
        return self.children_to_str(indent, '[&](auto &&T)', False)[:-1] + '(' + self.expression.to_str() + ");\n"

class ASTFunctionDefinition(ASTNodeWithChildren):
    function_name : str = ''
    function_return_type : str = ''
    is_const = False
    is_static = False
    is_declaration = False
    function_arguments : List[Tuple[str, str, str, str]]# = [] # (arg_name, default_value, type_, qualifier)
    first_named_only_argument = None
    last_non_default_argument : int
    class VirtualCategory(IntEnum):
        NO = 0
        NEW = 1
        OVERRIDE = 2
        ABSTRACT = 3
        ASSIGN = 4
        FINAL = 5
    virtual_category = VirtualCategory.NO
    scope : Scope
    member_initializer_list = ''

    def __init__(self, function_arguments = None, function_return_type = ''):
        super().__init__()
        self.function_arguments = function_arguments or []
        self.function_return_type = function_return_type
        self.scope = scope

    def serialize_to_dict(self, node_type = True):
        r = {}
        if node_type: # 'node_type' is inserted in dict before 'function_arguments' as this looks more logical in .11l_global_scope
            r['node_type'] = 'function'
        r['function_arguments'] = ['; '.join(arg) for arg in self.function_arguments]
        return r

    def deserialize_from_dict(self, d):
        self.function_arguments = [arg.split('; ') for arg in d['function_arguments']]

    def children_to_str(self, indent, t):
        if self.is_declaration:
            return pre_nl(self.tokeni) + ' ' * (indent*4) + t + ";\n"
        else:
            return super().children_to_str(indent, t)

    def to_str(self, indent):
        is_const = False
        if type(self.parent) == ASTTypeDefinition or '::' in self.function_name:
            if self.function_name == '': # this is constructor
                s = self.parent.type_name
            elif self.function_name == '(destructor)':
                s = '~' + self.parent.type_name
            elif self.function_name == 'String':
                s = 'operator String'
                is_const = True
            else:
                s = 'static '*self.is_static + ('auto' if self.function_return_type == '' else trans_type(self.function_return_type, self.scope, tokens[self.tokeni])) + ' ' + 's_'*self.is_static + \
                    {'()':'operator()', '[&]':'operator&', '<':'operator<', '==':'operator==', '!=':'operator!=', '+':'operator+', '-':'operator-', '*':'operator*', '/':'operator/'}.get(self.function_name, self.function_name)
                if self.function_name in tokenizer.all_operators:
                    is_const = True

            if self.virtual_category != self.VirtualCategory.NO:
                arguments = []
                for index, arg in enumerate(self.function_arguments):
                    if arg[2] == '': # if there is no type specified
                        raise Error('type should be specified for argument `' + arg[0] + '` [for virtual functions all arguments should have types]', tokens[self.tokeni])
                    else:
                        arguments.append(
                              ('' if '=' in arg[3] or '&' in arg[3] else 'const ')
                            + trans_type(arg[2].rstrip('?'), self.scope, tokens[self.tokeni]) + '* '*0 + ' '
                            + ('&' if '&' in arg[3] or '=' not in arg[3] else '')
                            + arg[0] + ('' if arg[1] == '' or index < self.last_non_default_argument else ' = ' + arg[1]))
                s = 'virtual ' + s + '(' + ', '.join(arguments) + ')' + ('', ' override', ' = 0', ' override', ' final')[self.virtual_category - 1]
                return pre_nl(self.tokeni) + ' ' * (indent*4) + s + ";\n" if self.virtual_category == self.VirtualCategory.ABSTRACT else self.children_to_str(indent, s)

        elif type(self.parent) != ASTProgram: # local functions [i.e. functions inside functions] are represented as C++ lambdas
            captured_variables = set()
            def gather_captured_variables(node):
                def f(sn : SymbolNode, capture_level = 0):
                    if sn.token.category == Token.Category.NAME:
                        if sn.token.value(source).startswith('@'):
                            by_ref = True # sn.parent and sn.parent.children[0] is sn and sn.parent.symbol.id[-1] == '=' and sn.parent.symbol.id not in ('==', '!=')
                            c = sn.token.value(source).count('@')
                            if c > capture_level:
                                t = sn.token.value(source)[c:]
                                if t.startswith('='):
                                    t = t[1:]
                                    by_ref = False
                                if not t.startswith(':'):
                                    captured_variables.add('this' if t == '' else '&'*by_ref + t)
                        elif sn.token.value(source) == '(.)':
                            captured_variables.add('this')
                    else:
                        for child in sn.children:
                            if child is not None:
                                f(child, capture_level + int(child.symbol is not None and child.symbol.id == '->'))

                node.walk_expressions(f)
                node.walk_children(gather_captured_variables)
            gather_captured_variables(self)

            arguments = []
            argument_types_are_specified = True
            for arg in self.function_arguments:
                if arg[2] == '': # if there is no type specified
                    arguments.append(('auto ' if '=' in arg[3] else 'const auto &') + arg[0] if arg[1] == '' else
                                          ('' if '=' in arg[3] else 'const ') + 'decltype(' + arg[1] + ') ' + arg[0] + ' = ' + arg[1])
                    argument_types_are_specified = False
                else:
                    tid = self.scope.parent.find(arg[2].rstrip('?'))
                    if tid is not None and len(tid.ast_nodes) and type(tid.ast_nodes[0]) == ASTTypeDefinition and (tid.ast_nodes[0].has_virtual_functions or tid.ast_nodes[0].has_pointers_to_the_same_type):
                        arguments.append('std::unique_ptr<' + arg[2].rstrip('?') + '> ' + arg[0] + ('' if arg[1] == '' else ' = ' + arg[1]))
                    else:
                        arguments.append(('' if '=' in arg[3] or '&' in arg[3] else 'const ') + trans_type(arg[2], self.scope, tokens[self.tokeni]) + ' ' + ('&'*((arg[2] not in ('Int', 'Float')) and ('=' not in arg[3]))) + arg[0] + ('' if arg[1] == '' else ' = ' + arg[1]))
            return self.children_to_str(indent, ('auto' if self.function_return_type == '' or not argument_types_are_specified else 'std::function<' + trans_type(self.function_return_type, self.scope, tokens[self.tokeni]) + '(' + ', '.join(trans_type(arg[2], self.scope, tokens[self.tokeni]) + '&'*('&' in arg[3]) for arg in self.function_arguments) + ')>') + ' ' + self.function_name
                + ' = [' + ', '.join(sorted(filter(lambda v: not '&'+v in captured_variables, captured_variables))) + ']('
                + ', '.join(arguments) + ')' + ('' if self.function_return_type == '' else ' -> ' + trans_type(self.function_return_type, self.scope, tokens[self.tokeni])))[:-1] + ";\n"

        else:
            s = ('auto' if self.function_return_type == '' else trans_type(self.function_return_type, self.scope, tokens[self.tokeni])) + ' ' + (self.function_name if self.function_name != '' else '_')

        #if len(self.function_arguments) == 0:
        #    return self.children_to_str(indent, s + '()' + ' const'*(self.is_const or is_const))

        templates = []
        arguments = []
        for index, arg in enumerate(self.function_arguments):
            if arg[2] == '': # if there is no type specified
                templates.append('typename T' + str(index + 1) + ('' if arg[1] == '' or index < self.last_non_default_argument else ' = decltype(' + arg[1] + ')'))
                arguments.append(('T' + str(index + 1) + ' ' if '=' in arg[3] else 'const '*(arg[3] != '&') + 'T' + str(index + 1) + ' &')
                    + arg[0] + ('' if arg[1] == '' or index < self.last_non_default_argument else ' = ' + arg[1]))
            else:
                tid = self.scope.parent.find(arg[2].rstrip('?'))
                if tid is not None and len(tid.ast_nodes) and type(tid.ast_nodes[0]) == ASTTypeDefinition and (tid.ast_nodes[0].has_virtual_functions or tid.ast_nodes[0].has_pointers_to_the_same_type):
                    arguments.append('std::unique_ptr<' + arg[2].rstrip('?') + '> '
                        #+ ('' if '=' in arg[3] else 'const ')
                        + arg[3] # add `&` if needed
                        + arg[0] + ('' if arg[1] == '' or index < self.last_non_default_argument else ' = ' + arg[1]))
                elif arg[2].endswith('?'):
                    ty = trans_type(arg[2].rstrip('?'), self.scope, tokens[self.tokeni])
                    if ty == 'File':
                        if arg[1] == 'nullptr': # dirty hack for pqmarkup.py [to fix error C2672 in `outfile.write(to_html(instr[range_el(i + 2, endqpos)], nullptr, i + 2));`] as I dislike `TextOutput = TextIO` or `from _11l import *` inside pqmarkup.py
                            ty = 'FileWr'
                        else:
                            templates.append('bool reading = false')
                            ty = 'TFile<reading>'
                    arguments.append(ty + '* '
                            + ('' if '=' in arg[3] else 'const ')
                            + arg[0] + ('' if arg[1] == '' or index < self.last_non_default_argument else ' = ' + arg[1]))
                else:
                    ty = trans_type(arg[2], self.scope, tokens[self.tokeni])
                    if ty == 'File':
                        templates.append('bool reading')
                        ty = 'TFile<reading>'
                    make_ref = arg[3] == '&'
                    arguments.append(
                        (('' if arg[3] == '=' else 'const ') + ty + ' ' + '&'*(arg[2] not in ('Int', 'Float') and arg[3] != '=') if arg[3] != '&' else ty + ' &')
                        + arg[0] + ('' if arg[1] == '' or index < self.last_non_default_argument else ' = ' + 'make_ref('*make_ref + arg[1] + ')'*make_ref))

        if self.member_initializer_list == '' and self.function_name == '' and type(self.parent) == ASTTypeDefinition:
            def is_const_var(var_name):
                for child in self.parent.children:
                    if type(child) == ASTExpression:
                        if child.expression.symbol.id == '=' and child.expression.children[0].symbol.id == '-' and len(child.expression.children[0].children) == 1:
                            if child.expression.children[0].children[0].token_str() == var_name:
                                return True
                    elif isinstance(child, ASTVariableDeclaration):
                        if child.is_const and var_name in child.vars:
                            return True
                return False

            i = 0
            while i < len(self.children):
                c = self.children[i]
                if isinstance(c, ASTExpression) and c.expression.symbol.id == '=' \
                        and c.expression.children[0].symbol.id == '.' \
                    and len(c.expression.children[0].children) == 1 \
                        and c.expression.children[0].children[0].token.category == Token.Category.NAME \
                      and ((c.expression.children[1].token.category == Token.Category.NAME
                        and c.expression.children[1].token_str() in (arg[0] for arg in self.function_arguments))
                         or is_const_var(c.expression.children[0].children[0].token_str())):

                    if self.scope.parent.ids.get(c.expression.children[0].children[0].token_str()) is None: # this member variable is defined in the base type/class
                        i += 1
                        continue

                    if self.member_initializer_list == '':
                        self.member_initializer_list = " :\n"
                    else:
                        self.member_initializer_list += ",\n"
                    ec1 = c.expression.children[1].to_str()
                    for index, arg in enumerate(self.function_arguments):
                        if arg[0] == ec1:
                            if arguments[index].startswith('std::unique_ptr<'):
                                ec1 = 'std::move(' + ec1 + ')'
                            break
                    self.member_initializer_list += ' ' * ((indent+1)*4) + c.expression.children[0].children[0].token_str() + '(' + ec1 + ')'
                    self.children.pop(i)
                    continue
                i += 1

        r = self.children_to_str(indent, ('template <' + ', '.join(templates) + '> ')*(len(templates) != 0) + s + '(' + ', '.join(arguments) + ')' + ' const'*(self.is_const or is_const) + self.member_initializer_list)

        if isinstance(self.parent, ASTTypeDefinition) and self.function_name in ('+', '-', '*', '/') and self.function_name + '=' not in self.parent.scope.ids and len(self.function_arguments) == 1:
            r += ' ' * (indent*4) + 'template <typename Ty> auto &operator' + self.function_name + "=(const Ty &t)\n"
            r += ' ' * (indent*4) + "{\n"
            r += ' ' * ((indent+1)*4) + '*this = *this ' + self.function_name + " t;\n"
            r += ' ' * ((indent+1)*4) + "return *this;\n"
            r += ' ' * (indent*4) + "}\n"

        return r

class ASTIf(ASTNodeWithChildren, ASTNodeWithExpression):
    else_or_elif : ASTNode = None
    likely = 0

    def walk_children(self, f):
        super().walk_children(f)
        if self.else_or_elif is not None:
            self.else_or_elif.walk_children(f)

    def to_str(self, indent):
        if self.likely == 0:
            if len(self.expression.children) == 2 and self.expression.children[0].is_var_decl:
                s = 'if (auto ' + self.expression.children[0].children[0].to_str() + '; ' + self.expression.to_str() + ')'
            else:
                s = 'if (' + self.expression.to_str() + ')'
        elif self.likely == 1:
            s = 'if (likely(' + self.expression.to_str() + '))'
        else:
            assert(self.likely == -1)
            s = 'if (unlikely(' + self.expression.to_str() + '))'
        return self.children_to_str_detect_single_stmt(indent, s, check_for_if = True) + (self.else_or_elif.to_str(indent) if self.else_or_elif is not None else '')

class ASTElseIf(ASTNodeWithChildren, ASTNodeWithExpression):
    else_or_elif : ASTNode = None

    def walk_children(self, f):
        super().walk_children(f)
        if self.else_or_elif is not None:
            self.else_or_elif.walk_children(f)

    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'else if (' + self.expression.to_str() + ')', check_for_if = True) + (self.else_or_elif.to_str(indent) if self.else_or_elif is not None else '')

class ASTElse(ASTNodeWithChildren):
    def to_str(self, indent):
        return self.children_to_str_detect_single_stmt(indent, 'else')

class ASTSwitch(ASTNodeWithExpression):
    class Case(ASTNodeWithChildren, ASTNodeWithExpression):
        def __init__(self):
            super().__init__()
            self.additional_expressions = []

    cases : List[Case]
    has_string_case = False

    def __init__(self):
        self.cases = []
        self.pre_nl = pre_nl()

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
                if child.token_str()[1:-1] == "\\":
                    return R"u'\\'"
                return "u'" + child.token_str()[1:-1].replace("'", R"\'") + "'"
            return child.to_str()

        r = self.pre_nl
        if self.has_string_case: # C++ does not support strings in case labels so insert if-elif-else chain in this case
            switch_expr = self.expression.to_str()
            if self.expression.token.category != Token.Category.NAME:
                switch_var = ''
                for c in switch_expr:
                    if 'a' <= c <= 'z' or 'A' <= c <= 'Z' or '0' <= c <= '9' or c == '_':
                        switch_var += c
                    else:
                        if len(switch_var) == 0 or switch_var[-1] != '_':
                            switch_var += '_'
                if switch_var != switch_expr:
                    r += ' ' * (indent*4) + 'auto ' + switch_var + ' = ' + switch_expr + ";\n"
                    switch_expr = switch_var

            for case in self.cases:
                if case.expression.token_str() in ('E', 'И', 'else', 'иначе'):
                    assert(id(case) == id(self.cases[-1]))
                    r += case.children_to_str_detect_single_stmt(indent, 'else')
                else:
                    cond_expr: str
                    if len(case.additional_expressions) == 0:
                        cond_expr = switch_expr + ' == ' + char_if_len_1(case.expression)
                    else:
                        cond_expr = 'in(' + switch_expr + ', make_tuple(' + char_if_len_1(case.expression)
                        for aexpr in case.additional_expressions:
                            cond_expr += ', ' + char_if_len_1(aexpr)
                        cond_expr += '))'
                    r += case.children_to_str_detect_single_stmt(indent, ('if' if id(case) == id(self.cases[0]) else 'else if') + ' (' + cond_expr + ')', check_for_if = True)
            return r

        r += ' ' * (indent*4) + 'switch (' + self.expression.to_str() + ")\n" + ' ' * (indent*4) + "{\n"
        for case in self.cases:
            cu = any(isinstance(child, ASTVariableDeclaration) for child in case.children) and case is not self.cases[-1]
            r += ' ' * (indent*4)
            if case.expression.token_str() in ('E', 'И', 'else', 'иначе'):
                r += 'default'
            else:
                r += 'case ' + char_if_len_1(case.expression)
                for aexpr in case.additional_expressions:
                    r += ":\n" + ' ' * (indent*4) + 'case ' + char_if_len_1(aexpr)
            r += ':' + ' {'*cu + "\n"
            for c in case.children:
                r += c.to_str(indent+1)
            r += ' ' * ((indent+1)*4) + 'break;' + ' }'*cu + "\n"
        return r + ' ' * (indent*4) + "}\n"

class ASTLoopWasNoBreak(ASTNodeWithChildren):
    def to_str(self, indent):
        return ''

class ASTLoop(ASTNodeWithChildren, ASTNodeWithExpression):
    loop_variable : str = None
    is_loop_variable_a_reference = False
    copy_loop_variable = False
    break_label_needed = -1
    has_continue = False
    has_L_index = False
    has_L_last_iteration = False
    has_L_remove_current_element_and_continue = False
    is_loop_variable_a_ptr = False
    was_no_break_node : ASTLoopWasNoBreak = None

    def has_L_was_no_break(self):
        return self.was_no_break_node is not None

    def to_str(self, indent):
        r = ''

        csv_read_column_names = []
        loop_auto = False
        if self.expression is not None and self.expression.token.category == Token.Category.NUMERIC_LITERAL:
            lv = self.loop_variable if self.loop_variable is not None else 'Lindex'
            tr = 'for (' + ('Int' if int_is_int64 else 'int') + ' ' + lv + ' = 0; ' + lv + ' < ' + self.expression.to_str() + '; ' + lv + '++)'
        else:
            if self.loop_variable is not None or (self.expression is not None and self.expression.symbol.id in ('..', '.<')):
                if self.loop_variable is not None and ',' in self.loop_variable:
                    tr = 'for (auto ' + '&&'*(not self.copy_loop_variable) + '[' + self.loop_variable + '] : ' + self.expression.to_str() + ')'
                else:
                    loop_auto = True
                    if self.loop_variable is not None and self.expression is not None and self.expression.function_call \
                            and self.expression.children[0].symbol.id == ':' and len(self.expression.children[0].children) == 2 \
                            and self.expression.children[0].children[0].token_str() == 'csv' \
                            and self.expression.children[0].children[1].token_str() in ('read', 'readf'):

                        def find_access_to_row_field_by_column_name(node):
                            def f(e: SymbolNode):
                                if e.symbol.id == '[' and not e.is_list and not e.is_dict and e.children[0].token_str() == self.loop_variable \
                                                                                          and e.children[1].token.category == Token.Category.STRING_LITERAL: # ]
                                    name_str = e.children[1].to_str()
                                    index_var_name = e.children[1].token_str()[1:-1].replace(' ', '_') + 'Index'
                                    csv_read_column_names.append((name_str, index_var_name))
                                    e.children[1].token.category = Token.Category.NAME
                                    e.children[1].token_str_override = index_var_name
                                for child in e.children:
                                    if child is not None:
                                        f(child)
                            node.walk_expressions(f)
                            node.walk_children(find_access_to_row_field_by_column_name)
                        find_access_to_row_field_by_column_name(self)

                    if len(csv_read_column_names):
                        expression_str = 'reader'
                    else:
                        expression_str = self.expression.to_str()
                    tr = 'for (auto ' + ('&' if self.is_loop_variable_a_reference else '&&'*(self.is_loop_variable_a_ptr or (not self.copy_loop_variable and not (
                        self.expression.symbol.id in ('..', '.<') or (self.expression.symbol.id == '(' and self.expression.children[0].symbol.id == '.' and self.expression.children[0].children[0].symbol.id == '(' and self.expression.children[0].children[0].children[0].symbol.id in ('..', '.<'))))) # ))
                                        ) + (self.loop_variable if self.loop_variable is not None else '__unused') + ' : ' + expression_str + ')'
            else:
                if self.expression is not None and self.expression.token.category == Token.Category.NAME:
                    l = tokens[self.tokeni].value(source)
                    raise Error('please write `' + l + ' ' + self.expression.token.value(source) + ' != 0` or `'
                                                 + l + ' 1..' + self.expression.token.value(source) + '` instead of `'
                                                 + l + ' ' + self.expression.token.value(source) + '`', Token(tokens[self.tokeni].start, self.expression.token.end, Token.Category.NAME))
                if self.expression is not None and len(self.expression.children) == 2 and self.expression.children[0].is_var_decl:
                    tr = 'for (auto ' + self.expression.children[0].children[0].to_str() + '; ' + self.expression.to_str() \
                               + '; ' + self.expression.children[0].children[0].to_str() + ')'
                else:
                    tr = 'while (' + (self.expression.to_str() if self.expression is not None else 'true') + ')'
        rr = self.children_to_str_detect_single_stmt(indent, tr)

        if self.has_L_remove_current_element_and_continue:
            if not loop_auto:
                raise Error('this kind of loop does not support `L.remove_current_element_and_continue`', tokens[self.tokeni])
            if self.has_L_last_iteration:
                raise Error('`L.last_iteration` can not be used with `L.remove_current_element_and_continue`', tokens[self.tokeni])
            if self.has_L_index:
                raise Error('`L.index` can not be used with `L.remove_current_element_and_continue`', tokens[self.tokeni]) # {
            rr = ' ' * (indent*4) + '{auto &&__range = ' + self.expression.to_str() + ";\n" \
               + ' ' * (indent*4) + "auto __end = __range.end();\n" \
               + ' ' * (indent*4) + "auto __dst = __range.begin();\n" \
               + self.children_to_str(indent, 'for (auto __src = __range.begin(); __src != __end;)', False,
                    add_at_beginning = ' ' * ((indent+1)*4) + 'auto &&'+ self.loop_variable + " = *__src;\n")[:-indent*4-2] \
               + ' ' * ((indent+1)*4) + "if (__dst != __src)\n" \
               + ' ' * ((indent+1)*4) + "    *__dst = std::move(*__src);\n" \
               + ' ' * ((indent+1)*4) + "++__dst;\n" \
               + ' ' * ((indent+1)*4) + "++__src;\n" \
               + ' ' * (indent*4) + "}\n" \
               + ' ' * (indent*4) + "__range.erase(__dst, __end);}\n"

        if self.has_L_last_iteration:
            if not loop_auto:
                raise Error('this kind of loop does not support `L.last_iteration`', tokens[self.tokeni])
            rr = ' ' * (indent*4) + '{auto &&__range = ' + self.expression.to_str() \
                + ";\n" + self.children_to_str(indent, 'for (auto __begin = __range.begin(), __end = __range.end(); __begin != __end;)', False,
                    add_at_beginning = ' ' * ((indent+1)*4) + 'auto &&'+ self.loop_variable + " = *__begin; ++__begin;\n")
        elif self.has_L_index and not (self.loop_variable is None and self.expression is not None and self.expression.token.category == Token.Category.NUMERIC_LITERAL):
            rr = self.children_to_str(indent, tr, False)

        if len(csv_read_column_names):
            self.expression.append_child(SymbolNode(Token(0, 0, Token.Category.NAME), "skip_first_row'"))
            sn = SymbolNode(Token(0, 0, Token.Category.CONSTANT), '0B')
            sn.symbol = symbol_table['(constant)']
            self.expression.append_child(sn)

            pr = ' ' * (indent*4) + "{\n" \
               + ' ' * (indent*4) + 'auto reader = ' + self.expression.to_str() + ";\n" \
               + ' ' * (indent*4) + "auto header = reader.read_current_row_as_str_array();\n"
            for name_str, index_var_name in csv_read_column_names:
                pr += ' ' * (indent*4) + 'int ' + index_var_name + ' = header.index(' + name_str + ");\n"
            rr = pr + rr
            rr += ' ' * (indent*4) + "}\n"

        if self.has_L_index and not (self.loop_variable is None and self.expression is not None and self.expression.token.category == Token.Category.NUMERIC_LITERAL):
            if self.has_continue:
                brace_pos = int(rr[0] == "\n") + indent*4 + len(tr) + 1
                rr = rr[:brace_pos+1] + rr[brace_pos:] # {
            r += ' ' * (indent*4) + '{' + ('Int' if int_is_int64 else 'int') + " Lindex = 0;\n" + rr[:-indent*4-2] + "} on_continue:\n"*self.has_continue + ' ' * ((indent+1)*4) + "Lindex++;\n" + ' ' * (indent*4) + "}}\n"
        else:
            r += rr

        if self.has_L_last_iteration: # {
            r = r[:-1] + "}\n"

        if self.has_L_was_no_break():
            curly_braces_needed = False
            for c in self.was_no_break_node.children:
                if isinstance(c, ASTVariableDeclaration):
                    curly_braces_needed = True
                    break
            s = ''
            for c in self.was_no_break_node.children:
                s += c.to_str(indent)
            if curly_braces_needed:
                r += ' ' * (indent*4) + "{\n" + s + ' ' * (indent*4) + "}\n"
            else:
                r += s

        if self.break_label_needed != -1:
            r += ' ' * (indent*4) + 'break_' + ('' if self.break_label_needed == 0 else str(self.break_label_needed)) + ":;\n"
        return r

    def walk_expressions(self, f):
        if self.expression is not None: f(self.expression)

    def walk_children(self, f):
        super().walk_children(f)
        if self.was_no_break_node is not None:
            self.was_no_break_node.walk_children(f)

class ASTContinue(ASTNode):
    token : Token

    def to_str(self, indent):
        n = self.parent
        while True:
            if type(n) == ASTLoop:
                n.has_continue = True
                break
            n = n.parent
            if n is None:
                raise Error('loop corresponding to this statement is not found', self.token)
        return ' ' * (indent*4) + 'goto on_'*n.has_L_index + "continue;\n"

break_label_index = -1

class ASTLoopBreak(ASTNode):
    loop_variable : str = ''
    loop_level = 0
    token : Token
    break_label = -1

    def set_break_label_needed(self):
        n = self.parent
        loop_level = 0
        while True:
            if type(n) == ASTLoopWasNoBreak:
                assert(type(n.parent) == ASTLoop)
                n = n.parent.parent
            if type(n) == ASTLoop:
                if loop_level == self.loop_level if self.loop_variable == '' else self.loop_variable == n.loop_variable:
                    if n.has_L_was_no_break() or loop_level > 0:
                        if n.break_label_needed == -1:
                            global break_label_index
                            break_label_index += 1
                            n.break_label_needed = break_label_index
                        self.break_label = n.break_label_needed
                        return
                    break
                loop_level += 1
            n = n.parent
            if n is None:
                raise Error('loop corresponding to this `' + '^'*self.loop_level + 'L' + ('(' + self.loop_variable + ')')*(self.loop_variable != '') + '.break` statement is not found', self.token)

        n = self.parent
        while True:
            if type(n) == ASTSwitch:
                n = n.parent
                while True:
                    if type(n) == ASTLoop:
                        if n.break_label_needed == -1:
                            break_label_index += 1
                            n.break_label_needed = break_label_index
                        self.break_label = n.break_label_needed
                        return
                    n = n.parent
            if type(n) == ASTLoop:
                break
            n = n.parent

    def to_str(self, indent):
        return ' ' * (indent*4) + ("break;\n" if self.break_label == -1 else 'goto break_' + ('' if self.break_label == 0 else str(self.break_label)) + ";\n")

class ASTLoopRemoveCurrentElementAndContinue(ASTNode):
    def to_str(self, indent):
        n = self.parent
        while True:
            if type(n) == ASTLoop:
                n.has_L_remove_current_element_and_continue = True
                break
            n = n.parent
        return ' ' * (indent*4) + "++__src;\n" \
             + ' ' * (indent*4) + "continue;\n"

class ASTReturn(ASTNodeWithExpression):
    def __init__(self):
        self.pre_nl = pre_nl()

    def to_str(self, indent):
        expr_str = ''
        if self.expression is not None:
            if self.expression.is_list and len(self.expression.children) == 0: # `R []`
                n = self.parent
                while type(n) != ASTFunctionDefinition:
                    n = n.parent
                if n.function_return_type == '':
                    raise Error('Function returning an empty array should have return type specified', self.expression.left_to_right_token())
                if not n.function_return_type.startswith('Array['): # ]
                    raise Error('Function returning an empty array should have an Array based return type', self.expression.left_to_right_token())
                expr_str = trans_type(n.function_return_type, self.expression.scope, self.expression.token) + '()'
            elif self.expression.function_call and self.expression.children[0].token_str() == 'Dict' and len(self.expression.children) == 1: # `R Dict()`
                n = self.parent
                while type(n) != ASTFunctionDefinition:
                    n = n.parent
                if n.function_return_type == '':
                    raise Error('Function returning an empty dict should have return type specified', self.expression.left_to_right_token())
                if not n.function_return_type.startswith('Dict['): # ]
                    raise Error('Function returning an empty dict should have a Dict based return type', self.expression.left_to_right_token())
                expr_str = trans_type(n.function_return_type, self.expression.scope, self.expression.token) + '()'
            else:
                expr_str = self.expression.to_str()
        return self.pre_nl + ' ' * (indent*4) + 'return' + (' ' + expr_str if expr_str != '' else '') + ";\n"

    def walk_expressions(self, f):
        if self.expression is not None: f(self.expression)

class ASTException(ASTNodeWithExpression):
    def __init__(self):
        self.pre_nl = pre_nl()

    def to_str(self, indent):
        return self.pre_nl + ' ' * (indent*4) + 'throw' + (' ' + self.expression.to_str() if self.expression is not None else '') + ";\n"

    def walk_expressions(self, f):
        if self.expression is not None: f(self.expression)

class ASTExceptionTry(ASTNodeWithChildren):
    def to_str(self, indent):
        return self.children_to_str(indent, 'try')

class ASTExceptionCatch(ASTNodeWithChildren):
    exception_object_type : str
    exception_object_name : str = ''

    def to_str(self, indent):
        if self.exception_object_type == '':
            return self.children_to_str(indent, 'catch (...)')
        return self.children_to_str(indent, 'catch (const ' + self.exception_object_type + '&' + (' ' + self.exception_object_name if self.exception_object_name != '' else '') + ')')

def named_tuple_members_and_constructor(indent, name, named_tuple_members):
    r = ''
    for member_type, member_name in named_tuple_members:
        r += ' ' * ((indent+1)*4) + member_type + ' ' + member_name + ";\n"
    r += "\n"
    r += ' ' * ((indent+1)*4) + name + '(' + ', '.join('const ' + ty + ' &' + na for ty, na in named_tuple_members) + ") :\n"
    r += ' ' * ((indent+2)*4) + ', '.join(na + '(' + na + ')' for ty, na in named_tuple_members) + " {}\n"
    return r

class ASTTypeDefinition(ASTNodeWithChildren):
    base_types : List[str]
    type_name : str
    constructors : List[ASTFunctionDefinition]
    has_virtual_functions = False
    has_pointers_to_the_same_type = False
    forward_declared_types : Set[str]
    serializable = False
    named_tuple_members: List[Tuple[str, str]]

    def __init__(self, constructors = None):
        super().__init__()
        self.base_types = []
        self.constructors = constructors or []
        if constructors is not None:
            self.scope = Scope(None) # needed for built-in types, e.g. `File(full_fname, ‘w’, encoding' ‘utf-8-sig’).write(...)`
        self.forward_declared_types = set()
        self.named_tuple_members = []

    def serialize_to_dict(self):
        return {'node_type': 'type', 'constructors': [c.serialize_to_dict(False) for c in self.constructors]}

    def deserialize_from_dict(self, d):
        for c_dict in d['constructors']:
            c = ASTFunctionDefinition()
            c.deserialize_from_dict(c_dict)
            self.constructors.append(c)

    def find_id_including_base_types(self, id):
        tid = self.scope.ids.get(id)
        if tid is None:
            for base_type_name in self.base_types:
                tid = self.scope.parent.find(base_type_name)
                assert(tid is not None and len(tid.ast_nodes) == 1)
                assert(isinstance(tid.ast_nodes[0], ASTTypeDefinition))
                tid = tid.ast_nodes[0].find_id_including_base_types(id)
                if tid is not None:
                    break
        return tid

    def set_serializable_to_children(self):
        self.serializable = True
        for c in self.children:
            if type(c) == ASTTypeDefinition:
                c.set_serializable_to_children()

    def to_str(self, indent):
        r = pre_nl(self.tokeni)
        base_types = []
        # if self.has_pointers_to_the_same_type:
        #     base_types += ['SharedObject']
        base_types += self.base_types
        r += ' ' * (indent*4) \
          + 'class ' + self.type_name + (' : ' + ', '.join(map(lambda c: 'public ' + c, base_types)) if len(base_types) else '') \
          + "\n" + ' ' * (indent*4) + "{\n"
        access_specifier_public = -1
        if len(self.named_tuple_members) > 0:
            r += ' ' * (indent*4) + "public:\n"
            access_specifier_public = 1
            r += named_tuple_members_and_constructor(indent, self.type_name, self.named_tuple_members) + "\n"
        for c in self.children:
            if c.access_specifier_public != access_specifier_public:
                r += ' ' * (indent*4) + ['protected', 'public'][c.access_specifier_public] + ":\n"
                access_specifier_public = c.access_specifier_public
            r += c.to_str(indent+1)
        if len(self.forward_declared_types):
            r = "\n".join(' ' * (indent*4) + 'class ' + t + ';' for t in self.forward_declared_types) + "\n\n" + r

        if self.serializable:
            r += "\n" + ' ' * ((indent+1)*4) + "void serialize(ldf::Serializer &s)\n" + ' ' * ((indent+1)*4) + "{\n"
            for c in self.children:
                if type(c) in (ASTVariableDeclaration, ASTVariableInitialization):
                    for var in c.vars:
                        r += ' ' * ((indent+2)*4) + 's(u"' + var + '", ' + (var if var != 's' else 'this->s') + ");\n"
            r += ' ' * ((indent+1)*4) + "}\n"

        return r + ' ' * (indent*4) + "};\n"

class ASTTypeAlias(ASTNode):
    name : str
    defining_type : str # this term is taken from C++ Standard (‘using identifier attribute-specifier-seqopt = defining-type-id ;’)
    template_params : List[str]
    named_tuple_members: List[Tuple[str, str]]

    def __init__(self):
        self.template_params = []
        self.named_tuple_members = []

    def to_str(self, indent):
        if len(self.named_tuple_members) > 0: # this is a named tuple
            r = self.pre_nl
            r += ' ' * (indent*4) + 'class ' + self.name + "\n"
            r += ' ' * (indent*4) + "{\n"
            r += ' ' * (indent*4) + "public:\n"
            r += named_tuple_members_and_constructor(indent, self.name, self.named_tuple_members)
            return r + ' ' * (indent*4) + "};\n"

        r = self.pre_nl + ' ' * (indent*4)
        if len(self.template_params):
            r += 'template <' + ', '.join(self.template_params) + '> '
        return r + 'using ' + self.name + ' = ' + self.defining_type + ";\n"

class ASTTypeEnum(ASTNode):
    enum_name : str
    enumerators : List[str]

    def __init__(self):
        super().__init__()
        self.enumerators = []

    def to_str(self, indent):
        r = ' ' * (indent*4) + 'enum class ' + self.enum_name + " {\n"
        for i in range(len(self.enumerators)):
            r += ' ' * ((indent+1)*4) + self.enumerators[i]
            if i < len(self.enumerators) - 1:
                r += ','
            r += "\n"
        return r + ' ' * (indent*4) + "};\n"

class ASTMain(ASTNodeWithChildren):
    found_reference_to_argv = False

    def to_str(self, indent):
        if importing_module:
            return ''
        if not self.found_reference_to_argv:
            return self.children_to_str(indent, 'int main()')
        return self.children_to_str(indent, 'int MAIN_WITH_ARGV()', add_at_beginning = ' ' * ((indent+1)*4) + "INIT_ARGV();\n\n")

class ASTIncludeFile(ASTNode):
    enclose_in_namespace = False

    def __init__(self, include_file_name):
        self.pre_nl = pre_nl()
        self.include_file_name = include_file_name

    def to_str(self, indent):
        r = self.pre_nl
        if self.enclose_in_namespace:
            r += ' ' * (indent*4) + 'namespace ' + os.path.basename(file_name)[:-4] + " {\n"
        r += ' ' * (indent*4) + '#include "' + self.include_file_name + '.hpp"\n'
        if self.enclose_in_namespace:
            r += ' ' * (indent*4) + "}\n"
            r += ' ' * (indent*4) + f"using namespace {os.path.basename(file_name)[:-4]};\n"
        return r

def type_of(sn):
    assert(sn.symbol.id == '.' and len(sn.children) == 2)
    if sn.children[0].symbol.id == '.':
        if len(sn.children[0].children) == 1:
            return None
        left = type_of(sn.children[0])
        if left is None: # `Array[Array[Array[String]]] table... table.last.append([...])`
            return None
    elif sn.children[0].symbol.id == '!' and sn.children[0].children[0].symbol.id == '.':
        assert(sn.children[0].postfix)
        if len(sn.children[0].children[0].children) == 1:
            return None
        left = type_of(sn.children[0].children[0])
        if left is None:
            return None
    elif sn.children[0].symbol.id == '[': # ]
        return None
    elif sn.children[0].symbol.id == '(': # )
        if not sn.children[0].function_call:
            return None
        if sn.children[0].children[0].symbol.id == '.':
            return None
        tid = sn.scope.find(sn.children[0].children[0].token_str())
        if tid is None:
            return None
        if type(tid.ast_nodes[0]) == ASTFunctionDefinition: # `input().split(...)`
            if tid.ast_nodes[0].function_return_type == '':
                return None
            type_name = tid.ast_nodes[0].function_return_type
            if type_name.startswith('Array['): # ]
                type_name = 'Array'
            tid = tid.ast_nodes[0].scope.find(type_name)
        else: # `Converter(habr_html, ohd).to_html(instr, outfilef)`
            type_name = sn.children[0].children[0].token_str()
        if not (tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) == ASTTypeDefinition):
            raise Error('unable to determine the return type of `' + sn.children[0].children[0].token_str() + '()`', sn.children[0].children[0].token)
        tid = tid.ast_nodes[0].scope.ids.get(sn.children[1].token_str())
        if not (tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) in (ASTVariableDeclaration, ASTVariableInitialization, ASTFunctionDefinition)):
            if type_name == 'auto&':
                return None
            raise Error('method `' + sn.children[1].token_str() + '` is not found in type `' + type_name + '`', sn.left_to_right_token())
        return tid.ast_nodes[0]
    elif sn.children[0].symbol.id == ':':
        if len(sn.children[0].children) == 2:
            return None # [-TODO-]
        assert(len(sn.children[0].children) == 1)
        tid = (scope if importing_module else global_scope).find(sn.children[0].children[0].token_str())
        if tid is None or len(tid.ast_nodes) != 1:
            raise Error('`' + sn.children[0].children[0].token_str() + '` is not found in global scope', sn.left_to_right_token()) # this error occurs without this code: ` or (self.token_str()[0].isupper() and self.token_str() != self.token_str().upper())`
        left = tid.ast_nodes[0]
    elif sn.children[0].token_str() == '@':
        s = sn.scope
        while True:
            if s.is_function:
                if s.is_lambda:
                    assert(s.node is None)
                    snp = s.parent.node
                else:
                    snp = s.node.parent
                if type(snp) == ASTFunctionDefinition:
                    if type(snp.parent) == ASTTypeDefinition:
                        fid = snp.parent.find_id_including_base_types(sn.children[1].token_str())
                        if fid is None:
                            raise Error('call of undefined method `' + sn.children[1].token_str() + '`', sn.left_to_right_token())
                        if len(fid.ast_nodes) > 1:
                            raise Error('methods\' overloading is not supported for now', sn.left_to_right_token())
                        f_node = fid.ast_nodes[0]
                        if type(f_node) == ASTFunctionDefinition:
                            return f_node
                break
            s = s.parent
            assert(s)
        return None
    elif sn.children[0].token_str().startswith('@'):
        return None # [-TODO-]
    else:
        if sn.children[0].token.category == Token.Category.STRING_LITERAL:
            tid = builtins_scope.ids.get('String')
            tid = tid.ast_nodes[0].scope.ids.get(sn.children[1].token_str())
            if not (tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) == ASTFunctionDefinition):
                raise Error('method `' + sn.children[1].token_str() + '` is not found in type `String`', sn.left_to_right_token())
            return tid.ast_nodes[0]

        c0 = sn.children[0]
        if c0.symbol.id == '!':
            assert(c0.postfix)
            c0 = c0.children[0]

        tid, s = sn.scope.find_and_return_scope(c0.token_str())
        if tid is None:
            raise Error('identifier is not found', c0.token)
        if len(tid.ast_nodes) != 1: # for `F f(active_window, s)... R s.find(‘.’) ? s.len`
            if tid.type != '' and s.is_function: # for `F nud(ASTNode self)... self.symbol.nud_bp`
                if '[' in tid.type: # ] # for `F decompress(Array[Int] &compressed)`
                    return None
                tid = s.find(tid.type)
                assert(tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) == ASTTypeDefinition)
                tid = tid.ast_nodes[0].scope.ids.get(sn.children[1].token_str())
                if not (tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) in (ASTVariableDeclaration, ASTVariableInitialization, ASTFunctionDefinition, ASTExpression)): # `ASTExpression` is needed to fix an error ‘identifier `disInter` is not found in `r`’ in '9.yopyra.py' (when there is no `disInter : float`)
                    raise Error('identifier `' + sn.children[1].token_str() + '` is not found in `' + c0.token_str() + '`', sn.children[1].token)
                if isinstance(tid.ast_nodes[0], ASTExpression):
                    return None
                return tid.ast_nodes[0]
            return None
        left = tid.ast_nodes[0]
        if type(left) == ASTLoop:
            return None

    if type(left) in (ASTTupleInitialization, ASTTupleAssignment):
        return None # [-TODO-]
    if type(left) == ASTTypeDefinition:
        tid = left.find_id_including_base_types(sn.children[1].token_str())
        if not (tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) in (ASTTypeDefinition, ASTFunctionDefinition, ASTTypeEnum)):
            raise Error('identifier `' + sn.children[1].token_str() + '` is not found in type `' + left.type_name + '`', sn.left_to_right_token())
        if type(tid.ast_nodes[0]) == ASTTypeEnum:
            return None # [-TODO-]
        return tid.ast_nodes[0]
    if type(left) not in (ASTVariableDeclaration, ASTVariableInitialization):
        raise Error('left type is `' + str(type(left)) + '`', sn.left_to_right_token())
    if left.type in ('V', 'П', 'var', 'пер', 'V?', 'П?', 'var?', 'пер?', 'V&', 'П&', 'var&', 'пер&'): # for `V selection_strings = ... selection_strings.map(...)`
        assert(type(left) == ASTVariableInitialization)
        if left.expression.function_call and left.expression.children[0].token.category == Token.Category.NAME and left.expression.children[0].token_str()[0].isupper(): # for `V n = Node()`
            tid = sn.scope.find(left.expression.children[0].token_str())
            assert(tid is not None and len(tid.ast_nodes) == 1)
            if type(tid.ast_nodes[0]) == ASTFunctionDefinition:
                return None
            assert(type(tid.ast_nodes[0]) == ASTTypeDefinition)
            tid = tid.ast_nodes[0].find_id_including_base_types(sn.children[1].token_str())
            if not (tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) in (ASTVariableDeclaration, ASTVariableInitialization, ASTFunctionDefinition, ASTExpression)): # `ASTExpression` is needed to fix an error ‘identifier `Vhor` is not found in type `Scene`’ in '9.yopyra.py' (when `Vhor = .look.pVectorial(.upCamara)`, i.e. when there is no `Vhor : Vector`)
                raise Error('identifier `' + sn.children[1].token_str() + '` is not found in type `' + left.expression.children[0].token_str() + '`', sn.left_to_right_token()) # error message example: method `remove` is not found in type `Array`
            if isinstance(tid.ast_nodes[0], ASTExpression):
                return None
            return tid.ast_nodes[0]
        if ((left.expression.function_call and left.expression.children[0].symbol.id == '.' and len(left.expression.children[0].children) == 2 and left.expression.children[0].children[1].token_str() in ('map', 'filter')) # for `V a = ....map(Int); a.sort(reverse' 1B)`
                or left.expression.is_list # for `V employees = [...]; employees.sort(key' e -> e.name)`
                or (left.expression.symbol.id == '*' and left.expression.children[0].is_list) # for `V tmp = [(0, 0)] * 0; tmp.sort(key' x -> x[0])`
                or (left.expression.function_call and left.expression.children[0].is_list)): # for `V f = [Int](); f.sort(reverse' 1B)`
            tid = builtins_scope.find('Array').ast_nodes[0].scope.ids.get(sn.children[1].token_str())
            if not (tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) in (ASTVariableDeclaration, ASTVariableInitialization, ASTFunctionDefinition)):
                raise Error('member `' + sn.children[1].token_str() + '` is not found in type `Array`', sn.left_to_right_token())
            return tid.ast_nodes[0]
        return None
    # if len(left.type_args): # `Array[String] ending_tags... ending_tags.append(‘</blockquote>’)`
    #     return None # [-TODO-]
    if left.type == 'T':
        return None
    tid = left.scope.find(left.type.rstrip('?'))
    if not (tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) == ASTTypeDefinition):
        if left.type.startswith('('): # )
            return None
        raise Error('type `' + left.type + '` is not found', sn.left_to_right_token())
    tid = tid.ast_nodes[0].scope.ids.get(sn.children[1].token_str())
    if not (tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) in (ASTVariableDeclaration, ASTVariableInitialization, ASTFunctionDefinition)):
        raise Error('member `' + sn.children[1].token_str() + '` is not found in type `' + left.type.rstrip('?') + '`', sn.left_to_right_token())
    return tid.ast_nodes[0]

# List of C++ keywords is taken from here[https://en.cppreference.com/w/cpp/keyword]
cpp_keywords = {'alignas', 'alignof', 'and', 'and_eq', 'asm', 'auto', 'bitand', 'bitor', 'bool', 'break', 'case', 'catch', 'char', 'char8_t', 'char16_t', 'char32_t', 'class', 'compl', 'concept', 'const',
    'consteval', 'constexpr', 'constinit', 'const_cast', 'continue', 'co_await', 'co_return', 'co_yield', 'decltype', 'default', 'delete', 'do', 'double', 'dynamic_cast', 'else', 'enum', 'explicit',
    'export', 'extern', 'false', 'float', 'for', 'friend', 'goto', 'if', 'inline', 'int', 'long', 'mutable', 'namespace', 'new', 'noexcept', 'not', 'not_eq', 'nullptr', 'operator', 'or', 'or_eq',
    'private', 'protected', 'public', 'reflexpr', 'register', 'reinterpret_cast', 'requires', 'return', 'short', 'signed', 'sizeof', 'static', 'static_assert', 'static_cast', 'struct', 'switch',
    'template', 'this', 'thread_local', 'throw', 'true', 'try', 'typedef', 'typeid', 'typename', 'union', 'unsigned', 'using', 'virtual', 'void', 'volatile', 'wchar_t', 'while', 'xor', 'xor_eq',
    'j0', 'j1', 'jn', 'y0', 'y1', 'yn', 'pascal', 'main', 'stat', 'SIZE', 'apply', 'INFINITY'}

russian_names = {
    'ввод':'input', 'вывод':'print', "кон'":"end'", 'вывод_эл':'print_elements', "разд'":"sep'", 'выход':'exit', 'скоп':'copy', 'перем':'move', 'сортй':'sorted', 'обратный':'reversed',
    'обмен':'swap', 'все':'all', 'любой':'any', 'сумма':'sum', 'нод':'gcd', 'шаг':'step', 'формат':'format',
    'Пуст':'Void', 'Бул':'Bool',
     'Цел': 'Int', 'Цел8':'Int8',  'Цел16': 'Int16',  'Цел32': 'Int32',  'Цел64': 'Int64',  'ДлЦел': 'Long',  'ЦелУкз': 'IntPtr',  'Размер': 'Size', 'Укз':'Ptr', 'как':'as',
    'БЦел':'UInt', 'Байт':'Byte', 'БЦел16':'UInt16', 'БЦел32':'UInt32', 'БЦел64':'UInt64', 'БДлЦел':'ULong', 'БЦелУкз':'UIntPtr', 'БРазмер':'USize', 'БолЦел':'BigInt',
    'Вещ':'Float', 'Вещ32':'Float32', 'Вещ64':'Float64',
    'Символ':'Char', 'код':'code', "код'":"code'", "цифра'":"digit'", "строка'":"string'",
     'е_цифра':'is_digit', 'е_буква':'is_alpha', 'е_нижрег':'is_lowercase', 'е_верхрег':'is_uppercase', 'нижрег':'lowercase', 'верхрег':'uppercase',
    'Строка':'String', 'длина':'len', 'пуста':'empty', 'пусто':'empty', 'послед':'last', 'начин_с':'starts_with', 'начин_на':'starts_with', 'закан_на':'ends_with',
     'колво':'count', 'только_цифры':'is_digit', 'только_буквы':'is_alpha', 'сократить':'trim',
     'найти':'find', 'найти_и':'findi', 'об_найти':'rfind', 'об_найти_и':'rfindi', 'заменить':'replace',
     'разделить':'split', 'разделить_п':'split_py', 'разбить':'split', 'разбить_п':'split_py', "групп_разделители'":"group_delimiters'",
    'Кортеж':'Tuple',
    'Массив':'Array', 'пуст':'empty', 'соединить':'join', 'применить':'map', 'фильтр':'filter', 'добавить':'append', 'извлечь':'pop', 'индекс':'index', 'сорт':'sort', 'сорт_диапазон':'sort_range', 'обратить':'reverse',
    'Словарь':'Dict', 'получить':'get', 'ключи':'keys', 'значения':'values',
    'Счётчик':'Counter', 'Счетчик':'Counter', 'Дробь':'Fraction', 'числитель':'numerator', 'знаменатель':'denominator',
    'Множество':'Set', 'доб':'add', 'удалить':'remove', 'отбросить':'discard', 'очистить':'clear',
    'Дек':'Deque', 'добавить_в_начало':'append_left', 'расширить':'extend', 'расширить_с_начала':'extend_left', 'вставить':'insert', 'извлечь_с_начала':'pop_left',
    'Файл':'File', 'прочитать':'read', 'прочитать_строку':'read_line', 'прочитать_строки':'read_lines', 'прочитать_байты':'read_bytes', 'записать':'write', 'записать_байты':'write_bytes',
    'минкуча':'minheap', 'макскуча':'maxheap', 'поместить':'push',
}

def next_token(): # why ‘next_token’: >[https://youtu.be/Nlqv6NtBXcA?t=1203]:‘we'll have an advance method which will fetch the next token’
    global token, tokeni, tokensn
    if token is None and tokeni != -1:
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
            elif token.category == Token.Category.FSTRING:
                key = '(fstring)'
            elif token.category == Token.Category.NAME:
                key = '(name)'
                if token.value(source) == '': # empty module name
                    pass
                elif token.value(source)[0] == '@':
                    if token.value(source)[1:2] == '=':
                        if token.value(source)[2:] in cpp_keywords:
                            tokensn.token_str_override = '@=_' + token.value(source)[2:] + '_'
                    elif token.value(source)[1:] in cpp_keywords:
                        tokensn.token_str_override = '@_' + token.value(source)[1:] + '_'
                elif token.value(source) in russian_names:
                    tokensn.token_str_override = russian_names[token.value(source)]
                elif token.value(source)[-1] == "'":
                    if token.value(source)[:-1] in cpp_keywords:
                        tokensn.token_str_override = '_' + token.value(source)[:-1] + "_'"
                elif token.value(source) in cpp_keywords:
                    tokensn.token_str_override = '_' + token.value(source) + '_'
            elif token.category == Token.Category.CONSTANT:
                key = '(constant)'
            elif token.category == Token.Category.STRING_CONCATENATOR:
                key = '(concat)'
            elif token.category == Token.Category.SCOPE_BEGIN:
                key = '{' # }
            elif token.category in (Token.Category.STATEMENT_SEPARATOR, Token.Category.SCOPE_END, Token.Category.FSTRING_END):
                key = ';'
            else:
                key = token.value(source)
                if key == '?' and source[token.end] in (' ', "\n") and source[token.start-1] != ' ': # for `(Char, [Int])?` (but `i?1` must be equivalent to `i ? 1`)
                    key = 'T? '
                elif key == ':' and source[token.start-1] == ' ' and source[token.end] != ' ': # for `Int :i1`
                    if tokens[tokeni-1].category not in (Token.Category.KEYWORD, Token.Category.OPERATOR): # for `I :argv.len == 1` and `L n !C :p`
                        pc = source[token.start-2] # [[
                        if pc.isalpha() or pc.isdigit() or pc in '_]': # in 11l: `I source[token.start-2]. {.is_alpha() | .is_digit() | (.) C ‘_]’}`
                            key = 'T :'
                # elif key == '&' and source[token.start-1] == ' ' and source[token.end] not in (' ', "\n"): # for `F (ArgTy &arg)`
                #     key = 'T &'
                # elif key == '=' and source[token.start-1] == ' ' and source[token.end] not in (' ', "\n"): # for `F (ArgTy =arg)`
                #     key = 'T ='
                elif key in ('&', '=') and source[token.start-1] == ' ' and source[token.end] not in (' ', "\n"): # for `F (ArgTy &arg)` and `F (ArgTy =arg)`
                    key = 'T ' + key
            tokensn.symbol = symbol_table[key]

def advance(value):
    if token.value(source) != value:
        raise Error('expected `' + value + '`', token)
    next_token()

def peek_token(how_much = 1):
    return tokens[tokeni+how_much] if tokeni+how_much < len(tokens) else Token()

# This implementation is based on [http://svn.effbot.org/public/stuff/sandbox/topdown/tdop-4.py]
def expression(rbp = 0):
    def check_tokensn():
        if tokensn is None:
            raise Error('unexpected end of source', Token(len(source), len(source), Token.Category.STATEMENT_SEPARATOR))
        if tokensn.symbol is None:
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

infix('[+]', 20); #infix('->', 15) # for `(0 .< h).map(_ -> [0] * @w [+] [1])`

#infix('?', 25) # based on C# operator precedence ([http://www.ecma-international.org/publications/files/ECMA-ST/Ecma-334.pdf])

infix('|', 30); infix('&', 40)

infix('==', 50); infix('!=', 50); infix('C', 50); infix('С', 50); infix('in', 50); infix('св', 50); infix('!C', 50); infix('!С', 50); infix('!in', 50); infix('!св', 50)

#infix('(concat)', 52) # `instr[prevci - 1 .< prevci]‘’prevc C ("/\\", "\\/")` = `(instr[prevci - 1 .< prevci]‘’prevc) C ("/\\", "\\/")`

infix('..', 55); infix('.<', 55); infix('.+', 55); infix('<.', 55); infix('<.<', 55) # `ch C ‘0’..‘9’` = `ch C (‘0’..‘9’)`
#postfix('..', 55)

infix('?', 57) # `source[s .< colon_pos ? j]` = `source[s .< (colon_pos ? j)]`

infix('<', 60); infix('<=', 60)
infix('>', 60); infix('>=', 60)

infix('[|]', 70); infix('(+)', 80); infix('[&]', 90)

infix('<<', 100); infix('>>', 100)

infix('+', 110); infix('-', 110)

infix('(concat)', 115) # `print(‘id = ’id+1)` = `print((‘id = ’id)+1)`, `str(c) + str(1-c)*charstack[0]` -> `String(c)‘’String(1 - c) * charstack[0]` = `String(c)‘’(String(1 - c) * charstack[0])`

infix('*', 120); infix('/', 120); infix('I/', 120); infix('Ц/', 120); infix('-I/', 120); infix('-Ц/', 120)
infix('%', 120); infix('-%', 120)

prefix('-', 130); prefix('+', 130); prefix('!', 130); prefix('~', 130); prefix('--', 130); prefix('++', 130); prefix('&', 130)

infix_r('^', 140)

symbol('.', 150); symbol(':', 150); symbol('.:', 150); symbol('[', 150); symbol('(', 150); symbol(')'); symbol(']'); postfix('T? ', 150); postfix('T :', 150); postfix('--', 150); postfix('++', 150); postfix('!', 150); postfix('.*', 150)
prefix('.', 150); prefix(':', 150); prefix('.:', 150); symbol('[%', 150); prefix('T &', 130); prefix('T =', 150) # ]

infix_r('=', 10); infix_r('+=', 10); infix_r('-=', 10); infix_r('*=', 10); infix_r('/=', 10); infix_r('I/=', 10); infix_r('Ц/=', 10); infix_r('%=', 10); infix_r('>>=', 10); infix_r('<<=', 10); infix_r('^=', 10)
infix_r('[+]=', 10); infix_r('[&]=', 10); infix_r('[|]=', 10); infix_r('(+)=', 10); infix_r('‘’=', 10)

symbol('(name)').nud = lambda self: self
symbol('(literal)').nud = lambda self: self
symbol('(constant)').nud = lambda self: self

#symbol('(.)').nud = lambda self: self

symbol('L.last_iteration').nud = lambda self: self
symbol('Ц.последняя_итерация').nud = lambda self: self
symbol('loop.last_iteration').nud = lambda self: self
symbol('цикл.последняя_итерация').nud = lambda self: self

symbol('T').nud = lambda self: self
symbol('Т').nud = lambda self: self
symbol('type').nud = lambda self: self
symbol('тип').nud = lambda self: self

symbol(';')
symbol(',')
symbol("',")

def led(self, left):
    self.append_child(left)
    global scope
    prev_scope = scope
    scope = Scope([])
    scope.parent = prev_scope
    scope.is_lambda = True
    tokensn.scope = scope
    for c in left.children if left.symbol.id == '(' else [left]: # )
        if not c.token_str()[0].isupper(): # for `((ASTNode, ASTNode) -> ASTNode) led` and `[String = ((Float, Float) -> Float)] b` (fix error 'redefinition of already defined identifier is not allowed')
            scope.add_name(c.token_str(), None)
    self.append_child(expression(self.symbol.led_bp))
    scope = prev_scope
    return self
symbol('->', 15).set_led_bp(15, led)

def led(self, left):
    self.append_child(left) # [(
    if token.value(source) not in (']', ')') and token.category != Token.Category.SCOPE_BEGIN:
        self.append_child(expression(self.symbol.led_bp))
    return self
symbol('..', 55).set_led_bp(55, led)

def led(self, left):
    if token.category == Token.Category.SCOPE_BEGIN:
        self.append_child(left)
        self.append_child(tokensn)
        if token.value(source) == '{': # } # if current token is a `{` then it is "with"-expression, but not "with"-statement
            next_token()
            self.append_child(expression())
            advance('}')
        return self
    if token.value(source) in ('.', '('): # ) # "with"-expression
        self.append_child(left)
        sn = SymbolNode(Token(token.start, token.end, Token.Category.SCOPE_BEGIN))
        sn.symbol = symbol_table['{'] # }
        self.append_child(sn)
        self.append_child(expression())
        return self
    if token.category != Token.Category.NAME:
        raise Error('expected an attribute name', token)
    self.append_child(left)
    self.append_child(tokensn)
    next_token()
    return self
symbol('.').led = led

def led(self, left):
    if token.category != Token.Category.NAME:
        raise Error('expected an attribute name', token)
    self.append_child(left)
    self.append_child(tokensn)
    next_token()
    return self
symbol('.:').led = led

def led(self, left):
    expr = expression(10 - 1)
    assert(expr.function_call)
    f = expr.children[0]
    expr.children[0] = SymbolNode(Token(token.start, token.start, Token.Category.DELIMITER))
    expr.children[0].symbol = symbol_table['.']
    expr.children[0].parent = f.parent
    expr.children[0].append_child(left)
    expr.children[0].append_child(f)

    sn = SymbolNode(Token(token.start, token.start, Token.Category.OPERATOR))
    sn.symbol = symbol_table['=']
    sn.append_child(left)
    sn.append_child(expr)
    return sn
symbol('.=', 10).led = led

class Module:
    scope : Scope

    def __init__(self, scope):
        self.scope = scope

modules : Dict[str, Module] = {}
builtin_modules : Dict[str, Module] = {}
used_builtin_modules_set = None

def find_module(name):
    if name in modules:
        return modules[name]
    return builtin_modules[name]

def led(self, left):
    if token.category != Token.Category.NAME and token.value(source) != '(' and token.category != Token.Category.STRING_LITERAL: # )
        raise Error('expected an identifier name or string literal', token)

    # Process module [transpile it if necessary and load it]
    global scope
    ma = scope.module_alias(left.token_str())
    if ma is not None:
        left.token_str_override = ma
    module_name = left.token_str().replace(':', '::')
    if module_name not in modules and module_name not in builtin_modules:
        module_file_name = os.path.join(os.path.dirname(file_name), module_name.replace('::', '/')).replace('\\', '/') # `os.path.join()` is needed for case when `os.path.dirname(file_name)` is empty string, `replace('\\', '/')` is needed for passing 'tests/parser/errors.txt'
        try:
            modulefstat = os.stat(module_file_name + '.11l')
        except FileNotFoundError:
            raise Error('can not import module `' + module_name + "`: file '" + module_file_name + ".11l' is not found", left.token)

        hpp_file_mtime = 0
        if os.path.isfile(module_file_name + '.hpp'):
            hpp_file_mtime = os.stat(module_file_name + '.hpp').st_mtime
        if hpp_file_mtime == 0 \
                or modulefstat.st_mtime       > hpp_file_mtime \
                or os.stat(__file__).st_mtime > hpp_file_mtime \
                or os.stat(os.path.dirname(__file__) + '/tokenizer.py').st_mtime > hpp_file_mtime \
                or not os.path.isfile(module_file_name + '.11l_global_scope'):
            module_source = open(module_file_name + '.11l', encoding = 'utf-8-sig').read()
            prev_scope = scope
            s = parse_and_to_str(tokenizer.tokenize(module_source), module_source, module_file_name + '.11l', True)
            open(module_file_name + '.hpp', 'w', encoding = 'utf-8-sig', newline = "\n").write(s) # utf-8-sig is for MSVC (fix of error C2015: too many characters in constant [`u'‘'`]) # ’
            modules[module_name] = Module(scope)
            assert(scope.is_function == False) # serializing `is_function` member variable is not necessary because it is always equal to `False`
            open(module_file_name + '.11l_global_scope', 'w', encoding = 'utf-8', newline = "\n").write(eldf.to_eldf(scope.serialize_to_dict()))
            scope = prev_scope
        else:
            module_scope = Scope(None)
            module_scope.deserialize_from_dict(eldf.parse(open(module_file_name + '.11l_global_scope', encoding = 'utf-8-sig').read()))
            modules[module_name] = Module(module_scope)
    elif module_name in builtin_modules and used_builtin_modules_set is not None:
        used_builtin_modules_set.add(module_name.split('::')[0])

    self.append_child(left)
    if token.category == Token.Category.STRING_LITERAL: # for `re:‘pattern’`
        self.append_child(SymbolNode(Token(token.start, token.start, Token.Category.NAME), symbol = symbol_table['(name)']))
        sn = SymbolNode(Token(token.start, token.start, Token.Category.DELIMITER))
        sn.symbol = symbol_table['('] # )
        sn.function_call = True
        sn.append_child(self)
        sn.children.append(None)
        sn.append_child(tokensn)
        next_token()
        return sn
    elif token.value(source) != '(': # )
        self.append_child(tokensn)
        next_token()
    else: # for `os:(...)` and `time:(...)`
        self.append_child(SymbolNode(Token(token.start, token.start, Token.Category.NAME), symbol = symbol_table['(name)']))
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
    if token.value(source) in ('V', 'П', 'var', 'пер'):
        self.is_var_decl = True
        next_token()

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
    if token.value(source)[0].isupper() or (token.value(source) == '(' and source[token.start+1].isupper()): # ) # type name must start with an upper case letter
        self.is_type = True
        while True:
            self.append_child(expression())
            if self.children[-1].token.category == Token.Category.NAME and self.children[-1].token_str().endswith("'"):
                self.append_child(expression())
            if token.value(source) != ',':
                break
            advance(',')
    else:
        self.append_child(expression()) # [
    advance(']')
    return self
symbol('[').led = led # ]

def led(self, left):
    self.append_child(left)
    self.append_child(expression()) # [
    advance(']')
    return self
symbol('[%').led = led

def nud(self):
    if token.value(source) == ']': # for `R []`
        self.is_list = True # [
        advance(']')
    else:
        self.append_child(expression())
        if self.children[-1].symbol.id == '=':
            self.is_dict = True
            if token.value(source) == ',':
                advance(',')
                while True: # [[
                    if token.value(source) == ']':
                        break
                    self.append_child(expression())
                    assert(self.children[-1].symbol.id == '=')
                    if token.value(source) != ',':
                        break
                    advance(',')
            advance(']')
        else:
            self.is_list = True
            if token.value(source) == ',':
                advance(',')
                while True: # [[
                    if token.value(source) == ']':
                        break
                    self.append_child(expression())
                    if self.children[-1].token.category == Token.Category.NAME and self.children[-1].token_str().endswith("'"):
                        self.append_child(expression())
                    if token.value(source) != ',':
                        break
                    advance(',')
            advance(']')
    return self
symbol('[').nud = nud # ]

def nud(self):
    while token.category != Token.Category.FSTRING_END:
        if token.category == Token.Category.STRING_LITERAL:
            self.append_child(tokensn)
            next_token()
        else:
            assert(token.category == Token.Category.SCOPE_BEGIN)
            next_token()
            self.append_child(expression())
            if token.category == Token.Category.STATEMENT_SEPARATOR:
                # next_token()
                # assert(token.category == Token.Category.STRING_LITERAL)
                self.append_child(tokensn)
                next_token()
            assert(token.category == Token.Category.SCOPE_END)
            next_token()
    next_token()
    return self
symbol('(fstring)').nud = nud

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
            if token.value(source) in ('<', '<=', '>', '>='):
                self.append_child(tokensn)
                next_token()
                self.children[-1].append_child(expression())
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

included_11l_files = set()

def parse_internal(this_node):
    global token, scope

    def new_scope(node, func_args = None, call_advance_scope_begin = True):
        if call_advance_scope_begin:
            advance_scope_begin()
        global scope
        prev_scope = scope
        scope = Scope(func_args)
        scope.parent = prev_scope
        scope.init_ids_type_node()
        scope.node = node
        tokensn.scope = scope # можно избавиться от этой строки, если не делать вызов next_token() в advance_scope_begin()
        node.scope = scope
        parse_internal(node)
        scope = prev_scope
        if token is not None:
            tokensn.scope = scope

    def expected_name(what_name):
        next_token()
        if token.category != Token.Category.NAME:
            raise Error('expected ' + what_name, token)
        token_value = tokensn.token_str()
        next_token()
        return token_value

    def is_tuple_assignment():
        if token.value(source) == '(':
            ti = 1
            while peek_token(ti).value(source) != ')':
                if peek_token(ti).value(source) in ('[', '.'): # ] # `(u[i], u[j]) = (u[j], u[i])`, `(.x, .y, .z) = (vx, vy, vz)`
                    return False
                ti += 1
            return peek_token(ti + 1).value(source) == '='
        return False

    access_specifier_private = False

    while token is not None:
        if token.value(source) == ':' and peek_token().value(source) in ('start', 'старт') and peek_token(2).value(source) == ':':
            node = ASTMain()
            next_token()
            next_token()
            advance(':')
            assert(token.category == Token.Category.STATEMENT_SEPARATOR)
            next_token()
            new_scope(node, [], False)

        elif token.value(source) == '.' and type(this_node) == ASTTypeDefinition:
            access_specifier_private = True
            next_token()
            continue

        elif (token.category == Token.Category.NAME and peek_token().value(source) == ':' and peek_token(2).value(source) == '=' and
              peek_token(3).category == Token.Category.NAME and peek_token(4).value(source) == ':') or (token.value(source) == ':' and peek_token(1).value(source) == '=' and
              peek_token(2).category == Token.Category.NAME and peek_token(3).value(source) == ':'):
            if token.value(source) == ':':
                m = ''
            else:
                m = tokensn.token_str()
                next_token()
            next_token()
            next_token()
            scope.module_aliases[m] = tokensn.token_str()
            next_token()
            advance(':')
            if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                next_token()
            continue

        elif tokensn.symbol is not None and tokensn.symbol.id == '-' and (peek_token().category == Token.Category.NAME and peek_token(2).value(source) == ':' and peek_token(3).category == Token.Category.STATEMENT_SEPARATOR
                                                                                                                         or peek_token().value(source) == ':' and peek_token(2).category == Token.Category.STATEMENT_SEPARATOR):
            if peek_token().value(source) == ':':
                del scope.module_aliases['']
            else:
                next_token()
                del scope.module_aliases[tokensn.token_str()]
            next_token()
            advance(':')
            if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                next_token()
            continue

        elif token.category == Token.Category.NAME and peek_token().value(source) == ':' and peek_token(2).value(source) == '*':
            node = ASTIncludeFile(tokensn.token_str())

            def include_file(file_path, namespace):
                global included_11l_files
                if file_path in included_11l_files:
                    return
                included_11l_files.add(file_path)
                module_source = open(file_path, encoding = 'utf-8-sig').read()
                s = "#pragma once\n\n"
                s += parse_and_to_str(tokenizer.tokenize(module_source), module_source, file_path, namespace, reset_scope = False)
                open(file_path.rsplit('.', 1)[0] + '.hpp', 'w', encoding = 'utf-8-sig', newline = "\n").write(s)

            path_name = os.path.join(os.path.dirname(file_name), node.include_file_name).replace('\\', '/')
            if os.path.isfile(path_name + '.11l'):
                include_file(path_name + '.11l', importing_module)
            else:
                if not os.path.isdir(path_name):
                    raise Error(f"cannot include '{node.include_file_name}'", token)

                if not importing_module:
                    node.enclose_in_namespace = True

                hpp = open(path_name + '.hpp', 'w', encoding = 'utf-8-sig', newline = "\n")
                for file_path in os.listdir(path_name):
                    if file_path.endswith('.11l'):
                        include_file(path_name + '/' + file_path, os.path.basename(file_name)[:-4])
                        hpp.write(f'#include "{path_name}/{file_path[:-4]}.hpp"\n')

            next_token()
            next_token()
            next_token()
            if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                next_token()

        elif token.category == Token.Category.KEYWORD:
            if token.value(source).startswith(('F', 'Ф', 'fn', 'фн')):
                node = ASTFunctionDefinition()
                if '.virtual.' in token.value(source) or \
                   '.виртуал.' in token.value(source):
                    subkw = token.value(source)[token.value(source).rfind('.')+1:]
                    if   subkw in ('new',      'новая'   ): node.virtual_category = node.VirtualCategory.NEW
                    elif subkw in ('override', 'переопр' ): node.virtual_category = node.VirtualCategory.OVERRIDE
                    elif subkw in ('abstract', 'абстракт'): node.virtual_category = node.VirtualCategory.ABSTRACT
                    elif subkw in ('assign',   'опред'   ): node.virtual_category = node.VirtualCategory.ASSIGN
                    elif subkw in ('final',    'финал'   ): node.virtual_category = node.VirtualCategory.FINAL
                elif token.value(source) in ('F.destructor', 'Ф.деструктор', 'fn.destructor', 'фн.деструктор'):
                    if type(this_node) != ASTTypeDefinition:
                        raise Error('destructor declaration allowed only inside types', token)
                    node.function_name = '(destructor)' # can not use `~` here because `~` can be an operator overload
                if '.const' in token.value(source) or \
                   '.конст' in token.value(source):
                    node.is_const = True

                next_token()
                if node.function_name != '(destructor)':
                    if token.category == Token.Category.NAME:
                        node.function_name = tokensn.token_str()
                        next_token()
                        while token.value(source) == '.':
                            node.function_name += '::' + expected_name('function name')
                    elif token.value(source) == ':' and peek_token().category == Token.Category.NAME:
                        node.is_static = True
                        next_token()
                        node.function_name = tokensn.token_str()
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
                    prev_type_name = ''
                    while token.value(source) != ')':
                        if token.value(source) == "',":
                            assert(node.first_named_only_argument is None)
                            node.first_named_only_argument = len(node.function_arguments)
                            next_token()
                            continue
                        type_ = '' # (
                        if not (token.value(source) in ('&', '=') or (token.category == Token.Category.NAME
                                                                      and (peek_token().value(source) in (',', ';', ')') or
                                                                          (peek_token().value(source) == '=') and source[peek_token().start-1:peek_token().end+1] in (' = ', " =\n")))):
                            type_ = expression().to_type_str()
                            if type_.startswith('Array['): # ]
                                type_ = type_[5:]
                        if type_ == '':
                            type_ = prev_type_name
                        qualifier = ''
                        if token.value(source) == '=':
                            qualifier = '='
                            next_token()
                        elif token.value(source) == '&':
                            qualifier = '&'
                            next_token()
                        if token.category != Token.Category.NAME:
                            raise Error('expected function\'s argument name', token)
                        func_arg_name = tokensn.token_str()
                        next_token()
                        if token.value(source) == '=':
                            next_token()
                            default = expression().to_str()
                            was_default_argument = True
                        else:
                            if was_default_argument and node.first_named_only_argument is None:
                                raise Error('non-default argument follows default argument', tokens[tokeni-1])
                            default = ''
                        node.function_arguments.append((func_arg_name, default, type_, qualifier)) # ((
                        if token.value(source) not in ',;)':
                            raise Error('expected `)`, `;` or `,` in function\'s arguments list', token)
                        if token.value(source) == ',':
                            next_token()
                            prev_type_name = type_
                        elif token.value(source) == ';':
                            next_token()
                            prev_type_name = ''

                    node.last_non_default_argument = len(node.function_arguments) - 1
                    while node.last_non_default_argument >= 0 and node.function_arguments[node.last_non_default_argument][1] != '':
                        node.last_non_default_argument -= 1

                    if node.function_name not in cpp_type_from_11l: # there was an error in line `String sitem` because of `F String()`
                        scope.add_function(node.function_name, node)

                    next_token()
                    if token.value(source) == '->':
                        next_token()
                        if token.value(source) in ('N', 'Н', 'null', 'нуль'):
                            node.function_return_type = token.value(source)
                            next_token()
                        elif token.value(source) == '&':
                            node.function_return_type = 'auto&'
                            next_token()
                        else:
                            node.function_return_type = expression().to_type_str()

                if node.virtual_category != node.VirtualCategory.ABSTRACT:
                    if token.category == Token.Category.SCOPE_BEGIN and source[peek_token().start : peek_token().start + 3] == '...':
                        advance_scope_begin()
                        advance('..')
                        advance('.')
                        assert(token.category == Token.Category.SCOPE_END)
                        next_token()
                        node.is_declaration = True
                    else:
                        new_scope(node, map(lambda arg: (arg[0], arg[2], arg[1]), node.function_arguments))
                else:
                    if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                        next_token()

            elif token.value(source) in ('T', 'Т', 'type', 'тип', 'T.serializable', 'Т.сериализуемый', 'type.serializable', 'тип.сериализуемый'):
                serializable = token.value(source) in ('T.serializable', 'Т.сериализуемый', 'type.serializable', 'тип.сериализуемый')
                node = ASTTypeDefinition()
                node.type_name = expected_name('type name')

                if token.value(source) in ('[', '='): # ] # this is a type alias
                    n = ASTTypeAlias()
                    n.name = node.type_name
                    n.pre_nl = pre_nl(node.tokeni)
                    node = n
                    scope.add_name(node.name, node)

                    prev_scope = scope
                    scope = Scope(None)
                    scope.parent = prev_scope

                    if token.value(source) == '[':
                        next_token()
                        while True:
                            if token.category == Token.Category.KEYWORD and token.value(source) in ('T', 'Т', 'type', 'тип'):
                                next_token()
                                assert(token.category == Token.Category.NAME)
                                scope.add_name(token.value(source), ASTTypeDefinition())
                                node.template_params.append('typename ' + token.value(source))
                            else:
                                expr = expression()
                                type_name = trans_type(expr.to_type_str(), scope, expr.left_to_right_token())
                                assert(token.category == Token.Category.NAME)
                                scope.add_name(token.value(source), ASTTypeDefinition()) # :(hack):
                                node.template_params.append(type_name + ' ' + token.value(source))
                            next_token()
                            if token.value(source) == ']':
                                next_token()
                                break
                            advance(',')
                    advance('=')

                    if token.value(source) == '(' and peek_token(2).value(source) != ',':
                        next_token()
                        while token.value(source) != ')':
                            expr = expression()
                            type_name = trans_type(expr.to_type_str(), scope, expr.left_to_right_token())
                            if token.category != Token.Category.NAME:
                                raise Error('expected named tuple member name', token)
                            node.named_tuple_members.append((type_name, tokensn.token_str()))
                            next_token()
                            if token.value(source) == ',':
                                next_token()
                        node.constructor = ASTFunctionDefinition([(m[1], '', m[0]) for m in node.named_tuple_members])
                        next_token()
                    else:
                        expr = expression()
                        node.defining_type = trans_type(expr.to_type_str(), scope, expr.left_to_right_token())

                    scope = prev_scope
                    if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                        next_token()
                else:
                    scope.add_name(node.type_name, node)

                    if token.value(source) == '(':
                        if peek_token().value(source) == '(': # base type is a named tuple
                            next_token()
                            next_token()
                            while token.value(source) != ')':
                                expr = expression()
                                type_name = trans_type(expr.to_type_str(), scope, expr.left_to_right_token())
                                if token.category != Token.Category.NAME:
                                    raise Error('expected named tuple member name', token)
                                node.named_tuple_members.append((type_name, tokensn.token_str()))
                                next_token()
                                if token.value(source) == ',':
                                    next_token()
                            node.constructors.append(ASTFunctionDefinition([(m[1], '', m[0]) for m in node.named_tuple_members]))
                            next_token()
                        else:
                            while True:
                                node.base_types.append(expected_name('base type name'))
                                if token.value(source) != ',':
                                    break
                        if token.value(source) != ')': # (
                            raise Error('expected `)`', token)
                        next_token()

                    new_scope(node)

                    if serializable:
                        node.set_serializable_to_children()

                    for child in node.children:
                        if type(child) == ASTFunctionDefinition and child.virtual_category != child.VirtualCategory.NO:
                            node.has_virtual_functions = True
                            break

            elif token.value(source) in ('T.enum', 'Т.перечисл', 'type.enum', 'тип.перечисл'):
                node = ASTTypeEnum()
                node.enum_name = expected_name('enum name')
                scope.add_name(node.enum_name, node)
                advance_scope_begin()

                while True:
                    if token.category != Token.Category.NAME:
                        raise Error('expected an enumerator name', token)
                    enumerator = token.value(source)
                    if not enumerator.isupper():
                        raise Error('enumerators must be uppercase', token)
                    next_token()
                    if token.value(source) == '=':
                        next_token()
                        enumerator += ' = ' + expression().to_str()
                    node.enumerators.append(enumerator)
                    if token.category == Token.Category.SCOPE_END:
                        next_token()
                        break
                    assert(token.category == Token.Category.STATEMENT_SEPARATOR)
                    next_token()

            elif token.value(source).startswith(('I', 'Е', 'if', 'если')):
                node = ASTIf()
                if '.' in token.value(source):
                    subkw = token.value(source)[token.value(source).find('.')+1:]
                    if subkw in ('likely', 'часто'):
                        node.likely = 1
                    else:
                        assert(subkw in ('unlikely', 'редко'))
                        node.likely = -1
                next_token()
                node.set_expression(expression())
                new_scope(node)

                n = node
                while token is not None and token.value(source) in ('E', 'И', 'else', 'иначе'):
                    if peek_token().value(source) in ('I', 'Е', 'if', 'если'):
                        n.else_or_elif = ASTElseIf()
                        n.else_or_elif.parent = n
                        n = n.else_or_elif
                        next_token()
                        next_token()
                        n.set_expression(expression())
                        new_scope(n)
                    if token is not None and token.value(source) in ('E', 'И', 'else', 'иначе') and not peek_token().value(source) in ('I', 'Е', 'if', 'если'):
                        n.else_or_elif = ASTElse()
                        n.else_or_elif.parent = n
                        next_token()
                        if token.category == Token.Category.SCOPE_BEGIN:
                            new_scope(n.else_or_elif)
                        else: # for support `I fs:is_dir(_fname) {...} E ...` (without this `else` only `I fs:is_dir(_fname) {...} E {...}` is allowed)
                            expr_node = ASTExpression()
                            expr_node.pre_nl = pre_nl()
                            expr_node.set_expression(expression())
                            expr_node.parent = n.else_or_elif
                            n.else_or_elif.children.append(expr_node)
                            if not (token is None or token.category in (Token.Category.STATEMENT_SEPARATOR, Token.Category.SCOPE_END)):
                                raise Error('expected end of statement', token)
                            if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                                next_token()
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
                        ts = case.expression.token_str()
                        if case.expression.token.category == Token.Category.STRING_LITERAL and not (len(ts) == 3 or (ts[:2] == '"\\' and len(ts) == 4)):
                            node.has_string_case = True
                        while token.value(source) == ',':
                            advance(',')
                            case.additional_expressions.append(expression())
                            case.additional_expressions[-1].ast_parent = case
                            ts = case.additional_expressions[-1].token_str()
                            if case.additional_expressions[-1].token.category == Token.Category.STRING_LITERAL and not (len(ts) == 3 or (ts[:2] == '"\\' and len(ts) == 4)):
                                node.has_string_case = True
                    new_scope(case)
                    node.cases.append(case)
                next_token()

            elif token.value(source) in ('L', 'Ц', 'loop', 'цикл'):
                if peek_token().value(source) == '(' and peek_token(4).value(source) == '.' and peek_token(4).start == peek_token(3).end:
                    assert(peek_token(5).value(source) in ('break', 'прервать'))
                    node = ASTLoopBreak()
                    node.token = token
                    next_token()
                    node.loop_variable = expected_name('loop variable')
                    advance(')')
                    advance('.')
                    next_token()
                    if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                        next_token()
                else:
                    node = ASTLoop()
                    next_token()
                    prev_scope = scope
                    scope = Scope(None)
                    scope.parent = prev_scope

                    if token.category == Token.Category.SCOPE_BEGIN:
                        node.expression = None
                    else:
                        if token.value(source) == '(' and token.start == tokens[tokeni-1].end:
                            if peek_token().value(source) == '&':
                                node.is_loop_variable_a_reference = True
                                next_token()
                            elif peek_token().value(source) == '=':
                                node.copy_loop_variable = True
                                next_token()
                            node.loop_variable = expected_name('loop variable')
                            while token.value(source) == ',':
                                if peek_token().value(source) == '=':
                                    node.copy_loop_variable = True
                                    next_token()
                                node.loop_variable += ', ' + expected_name('loop variable')
                            advance(')')
                        node.set_expression(expression())

                        if node.loop_variable is not None: # check if loop variable is a [smart] pointer
                            lv_node = None
                            if node.expression.token.category == Token.Category.NAME:
                                id = scope.find(node.expression.token_str())
                                if id is not None and len(id.ast_nodes) == 1:
                                    lv_node = id.ast_nodes[0]
                            elif node.expression.symbol.id == '.' and len(node.expression.children) == 2:
                                lv_node = type_of(node.expression)
                            if lv_node is not None and isinstance(lv_node, ASTVariableDeclaration) and lv_node.type == 'Array':
                                tid = scope.find(lv_node.type_args[0])
                                if tid is not None and len(tid.ast_nodes) == 1 and type(tid.ast_nodes[0]) == ASTTypeDefinition and tid.ast_nodes[0].has_virtual_functions:
                                    node.is_loop_variable_a_ptr = True

                            for loop_var in node.loop_variable.split(', '):
                                scope.add_name(loop_var, node)

                    new_scope(node)
                    scope = prev_scope

                    if token is not None and token.value(source) in ('L.was_no_break', 'Ц.не_был_прерван', 'loop.was_no_break', 'цикл.не_был_прерван'):
                        node.was_no_break_node = ASTLoopWasNoBreak()
                        node.was_no_break_node.parent = node
                        next_token()
                        new_scope(node.was_no_break_node)

            elif token.value(source) in ('L.continue', 'Ц.продолжить', 'loop.continue', 'цикл.продолжить'):
                node = ASTContinue()
                node.token = token
                next_token()
                if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            elif token.value(source) in ('L.break', 'Ц.прервать', 'loop.break', 'цикл.прервать'):
                node = ASTLoopBreak()
                node.token = token
                next_token()
                if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            elif token.value(source) in ('L.remove_current_element_and_continue', 'Ц.удалить_текущий_элемент_и_продолжить', 'loop.remove_current_element_and_continue', 'цикл.удалить_текущий_элемент_и_продолжить'):
                node = ASTLoopRemoveCurrentElementAndContinue()
                next_token()
                if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            elif token.value(source) in ('R', 'Р', 'return', 'вернуть'):
                node = ASTReturn()
                next_token()
                if token.category in (Token.Category.SCOPE_END, Token.Category.STATEMENT_SEPARATOR):
                    node.expression = None
                else:
                    node.set_expression(expression())
                if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            elif token.value(source) in ('X', 'Х', 'exception', 'исключение'):
                raise Error('exceptions of the first kind are not supported', token)

            elif token.value(source) in ('X.throw', 'Х.возбудить', 'exception.throw', 'исключение.возбудить'):
                node = ASTException()
                next_token()
                if token.category in (Token.Category.SCOPE_END, Token.Category.STATEMENT_SEPARATOR):
                    node.expression = None
                else:
                    node.set_expression(expression())
                if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

            elif token.value(source) in ('X.try', 'Х.контроль', 'exception.try', 'исключение.контроль'):
                node = ASTExceptionTry()
                next_token()
                new_scope(node)

            elif token.value(source) in ('X.catch', 'Х.перехват', 'exception.catch', 'исключение.перехват'):
                node = ASTExceptionCatch()
                if peek_token().category != Token.Category.SCOPE_BEGIN:
                    if peek_token().value(source) == '.':
                        next_token()
                    node.exception_object_type = expected_name('exception object type name').replace(':', '::')
                    if token.value(source) == ':':
                        next_token()
                        node.exception_object_type += '::' + token.value(source)
                        next_token()
                    if token.category == Token.Category.NAME:
                        node.exception_object_name = token.value(source)
                        next_token()
                else:
                    next_token()
                    node.exception_object_type = ''
                new_scope(node)

            else:
                raise Error('unrecognized statement started with keyword', token)

        elif token.value(source) == '^':
            node = ASTLoopBreak()
            node.token = token
            node.loop_level = 1
            next_token()
            while token.value(source) == '^':
                node.loop_level += 1
                next_token()

            if token.value(source) not in ('L.break', 'Ц.прервать', 'loop.break', 'цикл.прервать'):
                raise Error('expected `L.break`', token)

            next_token()
            if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                next_token()

        elif token.category == Token.Category.SCOPE_END:
            next_token()
            if token.category == Token.Category.STATEMENT_SEPARATOR and token.end == len(source): # Token.Category.EOF
                next_token()
                assert(token is None)
            return

        elif token.category == Token.Category.STATEMENT_SEPARATOR: # this `if` was added in revision 105[‘Almost complete work on tests/python_to_cpp/pqmarkup.txt’] in order to support `hor_col_align = S instr[j .< j + 2] {‘<<’ {‘left’}; ‘>>’ {‘right’}; ‘><’ {‘center’}; ‘<>’ {‘justify’}}` [there was no STATEMENT_SEPARATOR after this line of code]
            next_token()
            if token is not None:
                assert(token.category != Token.Category.STATEMENT_SEPARATOR)
            continue

        elif ((token.value(source) in ('V', 'П', 'var', 'пер') and peek_token().value(source) == '(') # ) # this is `V (a, b) = ...`
           or (token.value(source) == '-' and
        peek_token().value(source) in ('V', 'П', 'var', 'пер') and peek_token(2).value(source) == '(')): # this is `-V (a, b) = ...`
            node = ASTTupleInitialization()
            if token.value(source) == '-':
                node.is_const = True
                next_token()
            next_token()
            next_token()

            while True:
                assert(token.category == Token.Category.NAME)
                name = tokensn.token_str()
                node.dest_vars.append(name)
                scope.add_name(name, node)
                next_token()
                if token.value(source) == ')':
                    break
                advance(',')

            next_token()
            advance('=')
            node.set_expression(expression())

            if node.expression.function_call and node.expression.children[0].symbol.id == '.' \
                                         and len(node.expression.children[0].children) == 2   \
                                            and (node.expression.children[0].children[1].token_str() in ('split', 'split_py') # `V (name, ...) = ....split(...)` ~> `(V name, V ...) = ....split(...)` -> `...assign_from_tuple(name, ...);` (because `auto [name, ...] = ....split(...);` does not working)
                                             or (node.expression.children[0].children[1].token_str() == 'map' # for `V (w, h) = lines[1].split_py().map(i -> Int(i))`
                                             and node.expression.children[0].children[0].function_call)
                                             and node.expression.children[0].children[0].children[0].symbol.id == '.'
                                         and len(node.expression.children[0].children[0].children[0].children) == 2
                                             and node.expression.children[0].children[0].children[0].children[1].token_str() in ('split', 'split_py')):
                # n = node
                # node = ASTTupleAssignment()
                # for dv in n.dest_vars:
                #     node.dest_vars.append((dv, True))
                # node.set_expression(n.expression)
                node.bind_array = True

            if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                next_token()

        elif is_tuple_assignment(): # this is `(a, b) = ...` or `(a, V b) = ...` or `(V a, b) = ...`
            node = ASTTupleAssignment()
            next_token()

            while True:
                if token.category != Token.Category.NAME:
                    raise Error('expected variable name', token)

                add_var = False
                if token.value(source) in ('V', 'П', 'var', 'пер'):
                    add_var = True
                    next_token()
                    assert(token.category == Token.Category.NAME)

                name = tokensn.token_str()
                node.dest_vars.append((name, add_var))
                if add_var:
                    scope.add_name(name, node)
                next_token() # (

                if token.value(source) == ')':
                    break
                advance(',')

            next_token()
            advance('=')
            node.set_expression(expression())

            if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                next_token()

        elif ((token.value(source) in ('V', 'П', 'var', 'пер') and peek_token().category == Token.Category.SCOPE_BEGIN) # this is `V {v1 = ...; v2 = ...; ...}`
           or (token.value(source) == '-' and
        peek_token().value(source) in ('V', 'П', 'var', 'пер') and peek_token(2).category == Token.Category.SCOPE_BEGIN)): # this is `-V {v1 = ...; v2 = ...; ...}`
            first_pre_nl = pre_nl()
            is_const = False
            if token.value(source) == '-':
                is_const = True
                next_token()
            type_token = token
            next_token()
            next_token()

            first = True
            while True:
                node = ASTVariableInitialization()
                node.pre_nl = first_pre_nl if first else pre_nl()
                first = False

                assert(token.category == Token.Category.NAME)
                var_name = tokensn.token_str()
                next_token()
                advance('=')

                node.is_const = is_const
                node.vars = [var_name]
                node.set_expression(expression())
                node.type = type_token.value(source)
                node.type_token = type_token
                node.type_args = []

                scope.add_name(var_name, node)

                node.parent = this_node
                this_node.children.append(node)

                if token.category != Token.Category.STATEMENT_SEPARATOR:
                    assert(token.category == Token.Category.SCOPE_END)
                    next_token()
                    break

                next_token()

            continue

        else:
            npre_nl = pre_nl()
            node_expression = expression()
            if node_expression.symbol.id == '.' and node_expression.children[1].token.category == Token.Category.SCOPE_BEGIN and node_expression.children[1].token.value(source) not in ('{', '.', '('): # )} # this is a "with"-statement
                node = ASTWith()
                node.set_expression(node_expression.children[0])
                new_scope(node)
            else:
                if node_expression.symbol.id == '&' and node_expression.children[0].token.category == Token.Category.NAME and node_expression.children[1].token.category == Token.Category.NAME: # this is a reference declaration (e.g. `Symbol& symbol`)
                    node = ASTVariableDeclaration()
                    node.is_reference = True
                    node.vars = [node_expression.children[1].token_str()]
                    node.type = node_expression.children[0].token_str()
                    node.type_token = node_expression.token
                    node.type_args = []
                    scope.add_name(node.vars[0], node)
                elif token.category == Token.Category.NAME and tokens[tokeni-1].category != Token.Category.SCOPE_END:
                    var_name = tokensn.token_str()
                    next_token()
                    if token.value(source) == '=':
                        next_token()
                        node = ASTVariableInitialization()
                        node.set_expression(expression())
                        if node_expression.token.value(source) not in ('V', 'П', 'var', 'пер'):
                            if node_expression.token.value(source) in ('V?', 'П?', 'var?', 'пер?'):
                                node.is_ptr = True
                                node.nullable = True
                            else:
                                id = scope.find(node_expression.token_str())
                                if id is not None and len(id.ast_nodes) != 0:
                                    if type(id.ast_nodes[0]) == ASTTypeDefinition and (id.ast_nodes[0].has_virtual_functions or id.ast_nodes[0].has_pointers_to_the_same_type):
                                        node.is_ptr = True
                        elif node.expression.function_call and node.expression.children[0].token.category == Token.Category.NAME and node.expression.children[0].token_str()[0].isupper(): # for `V animal = Sheep(); animal.say()` -> `...; animal->say();`
                            id = scope.find(node.expression.children[0].token_str())
                            if not (id is not None and len(id.ast_nodes) != 0):
                                raise Error('identifier `' + node.expression.children[0].token_str() + '` is not found', node.expression.children[0].token)
                            if type(id.ast_nodes[0]) == ASTTypeDefinition: # support for functions beginning with an uppercase letter (e.g. Extract_Min)
                                if id.ast_nodes[0].has_virtual_functions or id.ast_nodes[0].has_pointers_to_the_same_type:
                                    node.is_ptr = True
                                # elif id.ast_nodes[0].has_pointers_to_the_same_type:
                                #     node.is_shared_ptr = True
                        node.vars = [var_name]
                    else:
                        node = ASTVariableDeclaration()
                        id = scope.find(node_expression.token_str().rstrip('?'))
                        if id is not None:
                            assert(len(id.ast_nodes) == 1)
                            if type(id.ast_nodes[0]) not in (ASTTypeDefinition, ASTTypeEnum, ASTTypeAlias):
                                raise Error('identifier is of type `' + type(id.ast_nodes[0]).__name__ + '` (should be ASTTypeDefinition, ASTTypeEnum, or ASTTypeAlias)', node_expression.token) # this error was in line `String sitem` because of `F String()`
                            if type(id.ast_nodes[0]) == ASTTypeDefinition:
                                if id.ast_nodes[0].has_virtual_functions or id.ast_nodes[0].has_pointers_to_the_same_type:
                                    node.is_ptr = True
                                # elif id.ast_nodes[0].has_pointers_to_the_same_type:
                                #     node.is_shared_ptr = True
                        node.vars = [var_name]
                        while token.value(source) == ',':
                            node.vars.append(expected_name('variable name'))
                    node.type = node_expression.token_str()
                    if node.type == '-' and len(node_expression.children) == 1:
                        node.is_const = True
                        node_expression = node_expression.children[0]
                        node.type = node_expression.token_str()
                    is_nullable = False
                    if node.type == '?' and len(node_expression.children) == 1:
                        is_nullable = True
                        node_expression = node_expression.children[0]
                        node.type = node_expression.token_str()
                    if node.type == ':' and len(node_expression.children) == 1:
                        node.is_static = True
                        node_expression = node_expression.children[0]
                        node.type = node_expression.token_str()
                    node.type_token = node_expression.token
                    node.type_args = []
                    if node.type == '[': # ]
                        if node_expression.is_dict:
                            assert(len(node_expression.children) == 1)
                            node.type = 'Dict'
                            node.type_args = [node_expression.children[0].children[0].to_type_str(), node_expression.children[0].children[1].to_type_str()]
                        elif node_expression.is_list:
                            if len(node_expression.children) == 3 and node_expression.children[1].token_str() in ("len'", "max_len'"):
                                node.type = 'ArrayFixLen' if node_expression.children[1].token_str() == "len'" else 'ArrayMaxLen'
                                node.type_args = [node_expression.children[0].to_type_str(), '__EXPRESSION__(' + node_expression.children[2].to_str() + ')']
                            else:
                                assert(len(node_expression.children) == 1)
                                node.type = 'Array'
                                node.type_args = [node_expression.children[0].to_type_str()]
                        else:
                            assert(node_expression.is_type)
                            node.type = node_expression.children[0].token_str()
                            if node.type == 'Array' and len(node_expression.children) == 4 and node_expression.children[2].token_str() in ("len'", "max_len'"):
                                node.type = 'ArrayFixLen' if node_expression.children[2].token_str() == "len'" else 'ArrayMaxLen'
                                node.type_args = [node_expression.children[1].to_type_str(), '__EXPRESSION__(' + node_expression.children[3].to_str() + ')']
                            else:
                                for i in range(1, len(node_expression.children)):
                                    node.type_args.append(node_expression.children[i].to_type_str())
                    elif node.type == '(': # )
                        if len(node_expression.children) == 1 and node_expression.children[0].symbol.id == '->':
                            node.function_pointer = True
                            c0 = node_expression.children[0]
                            assert(c0.children[1].token.category == Token.Category.NAME or c0.children[1].token_str() in ('N', 'Н', 'null', 'нуль'))
                            node.type = c0.children[1].token_str() # return value type
                            if c0.children[0].token.category == Token.Category.NAME:
                                node.type_args.append(c0.children[0].token_str())
                            elif c0.children[0].symbol.id == '[': # ]
                                node.type_args.append(c0.children[0].to_type_str())
                            else:
                                assert(c0.children[0].symbol.id == '(') # )
                                for child in c0.children[0].children:
                                    assert(child.token.category == Token.Category.NAME)
                                    node.type_args.append(child.token_str())
                        else: # this is a tuple
                            for child in node_expression.children:
                                node.type_args.append(child.to_type_str())
                            node.type = '(' + ', '.join(node.type_args) + ')'
                            node.type_args.clear()
                    elif node.type == '.':
                        node.type = node_expression.to_str()
                    if is_nullable:
                        if len(node.type_args):
                            node.type += '[' + ', '.join(node.type_args) + ']?'
                            node.type_args.clear()
                        else:
                            node.type += '?'
                    if not (node.type[0].isupper() or node.type[0] == '(' or node.type in ('var', 'пер')): # )
                        raise Error('type name must start with an upper case letter', node.type_token)
                    for var in node.vars:
                        scope.add_name(var, node)
                    if type(this_node) == ASTTypeDefinition and this_node.type_name == node.type.rstrip('?'):
                        this_node.has_pointers_to_the_same_type = True
                        node.is_ptr = True # node.is_shared_ptr = True
                else:
                    node = ASTExpression()
                    node.set_expression(node_expression)
                    if isinstance(this_node, ASTTypeDefinition) and node_expression.symbol.id == '=': # fix error ‘identifier `disInter` is not found in `r`’ in '9.yopyra.py'
                        c0 = node_expression.children[0]
                        if c0.symbol.id == '-' and len(c0.children) == 1:
                            if c0.children[0].symbol.id == ':' and len(c0.children[0].children) == 1:
                                name = c0.children[0].children[0].token_str()
                            else:
                                name = c0.children[0].token_str()
                        elif c0.symbol.id == ':' and len(c0.children) == 1:
                            name = c0.children[0].token_str()
                        else:
                            name = c0.token_str()
                        scope.add_name(name, node)

                node.pre_nl = npre_nl

                if not (token is None or token.category in (Token.Category.STATEMENT_SEPARATOR, Token.Category.SCOPE_END) or tokens[tokeni-1].category == Token.Category.SCOPE_END):
                    raise Error('expected end of statement', token)
                if token is not None and token.category == Token.Category.STATEMENT_SEPARATOR:
                    next_token()

        if access_specifier_private:
            node.access_specifier_public = 0
            access_specifier_private = False
        node.parent = this_node
        this_node.children.append(node)

    return

tokens    = []
source    = ''
tokeni    = -1
token     = Token(0, 0, Token.Category.STATEMENT_SEPARATOR)
#scope     = Scope(None)
#tokensn   = SymbolNode(token)
file_name = ''
importing_module = False

def token_to_str(token_str_override, token_category = Token.Category.STRING_LITERAL):
    return SymbolNode(Token(0, 0, token_category), token_str_override).to_str()

builtins_scope = Scope(None)
scope = builtins_scope
global_scope : Scope
tokensn   = SymbolNode(token)
f = ASTFunctionDefinition([('object', token_to_str('‘’'), ''), ('end', token_to_str(R'"\n"'), 'String'), ('flush', token_to_str('0B', Token.Category.CONSTANT), 'Bool')])
f.first_named_only_argument = 1
builtins_scope.add_function('print', f)
f = ASTFunctionDefinition([('object', token_to_str('‘’'), ''), ('sep', token_to_str('‘ ’'), 'String'),
                                                               ('end', token_to_str(R'"\n"'), 'String'), ('flush', token_to_str('0B', Token.Category.CONSTANT), 'Bool')])
f.first_named_only_argument = 1
builtins_scope.add_function('print_elements', f)
builtins_scope.add_function('input', ASTFunctionDefinition([('prompt', token_to_str('‘’'), 'String')], 'String'))
builtins_scope.add_function('assert', ASTFunctionDefinition([('expression', '', 'Bool'), ('message', token_to_str('‘’'), 'String')]))
builtins_scope.add_function('exit', ASTFunctionDefinition([('arg', '0', '')]))
builtins_scope.add_function('swap', ASTFunctionDefinition([('a', '', '', '&'), ('b', '', '', '&')]))
builtins_scope.add_function('zip', ASTFunctionDefinition([('iterable1', '', ''), ('iterable2', '', ''), ('iterable3', token_to_str('N', Token.Category.CONSTANT), '')]))
builtins_scope.add_function('all', ASTFunctionDefinition([('iterable', '', '')]))
builtins_scope.add_function('any', ASTFunctionDefinition([('iterable', '', '')]))
builtins_scope.add_function('cart_product', ASTFunctionDefinition([('iterable1', '', ''), ('iterable2', '', ''), ('iterable3', token_to_str('N', Token.Category.CONSTANT), ''), ('iterable4', token_to_str('N', Token.Category.CONSTANT), '')]))
builtins_scope.add_function('multiloop', ASTFunctionDefinition([('iterable1', '', ''), ('iterable2', '', ''), ('function', '', ''), ('optional', token_to_str('N', Token.Category.CONSTANT), '')]))
builtins_scope.add_function('multiloop_filtered', ASTFunctionDefinition([('iterable1', '', ''), ('iterable2', '', ''), ('filter_function', '', ''), ('function', '', ''), ('optional', token_to_str('N', Token.Category.CONSTANT), '')]))
builtins_scope.add_function('sum', ASTFunctionDefinition([('iterable', '', '')]))
builtins_scope.add_function('product', ASTFunctionDefinition([('iterable', '', '')]))
builtins_scope.add_function('enumerate', ASTFunctionDefinition([('iterable', '', ''), ('start', '0', 'Int')]))
builtins_scope.add_function('sorted', ASTFunctionDefinition([('iterable', '', ''), ('key', token_to_str('N', Token.Category.CONSTANT), ''), ('reverse', token_to_str('0B', Token.Category.CONSTANT), 'Bool')]))
builtins_scope.add_function('tuple_sorted', ASTFunctionDefinition([('tuple', '', ''), ('key', token_to_str('N', Token.Category.CONSTANT), ''), ('reverse', token_to_str('0B', Token.Category.CONSTANT), 'Bool')]))
builtins_scope.add_function('cmp_to_key', ASTFunctionDefinition([('func', '', '')]))
builtins_scope.add_function('reversed', ASTFunctionDefinition([('iterable', '', '')]))
builtins_scope.add_function('min', ASTFunctionDefinition([('arg1', '', ''), ('arg2', token_to_str('N', Token.Category.CONSTANT), ''), ('arg3', token_to_str('N', Token.Category.CONSTANT), ''), ('arg4', token_to_str('N', Token.Category.CONSTANT), '')]))
builtins_scope.add_function('max', ASTFunctionDefinition([('arg1', '', ''), ('arg2', token_to_str('N', Token.Category.CONSTANT), ''), ('arg3', token_to_str('N', Token.Category.CONSTANT), ''), ('arg4', token_to_str('N', Token.Category.CONSTANT), '')]))
builtins_scope.add_function('divmod', ASTFunctionDefinition([('x', '', ''), ('y', '', '')]))
builtins_scope.add_function('factorial', ASTFunctionDefinition([('x', '', '')]))
builtins_scope.add_function('gcd', ASTFunctionDefinition([('a', '', ''), ('b', '', '')]))
builtins_scope.add_function('hex', ASTFunctionDefinition([('x', '', '')]))
builtins_scope.add_function('bin', ASTFunctionDefinition([('x', '', '')]))
builtins_scope.add_function('copy', ASTFunctionDefinition([('object', '', '')]))
builtins_scope.add_function('move', ASTFunctionDefinition([('object', '', '')]))
builtins_scope.add_function('hash', ASTFunctionDefinition([('object', '', '')]))
builtins_scope.add_function('rotl', ASTFunctionDefinition([('value', '', 'Int'), ('shift', '', 'Int')]))
builtins_scope.add_function('rotr', ASTFunctionDefinition([('value', '', 'Int'), ('shift', '', 'Int')]))
builtins_scope.add_function('bsr', ASTFunctionDefinition([('x', '', '')]))
builtins_scope.add_function('bsf', ASTFunctionDefinition([('x', '', '')]))
builtins_scope.add_function('round', ASTFunctionDefinition([('number', '', 'Float'), ('ndigits', '0', '')]))
builtins_scope.add_function('sleep', ASTFunctionDefinition([('secs', '', 'Float')]))
builtins_scope.add_function('ceil',  ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('ceili', ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('floor', ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('floori',ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('trunc', ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('fract', ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('modf',  ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('fmod',  ASTFunctionDefinition([('x', '', 'Float'), ('y', '', 'Float')]))
builtins_scope.add_function('wrap',  ASTFunctionDefinition([('x', '', 'Float'), ('min_value', '', 'Float'), ('max_value', '', 'Float')]))
builtins_scope.add_function('abs',   ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('exp',   ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('log',   ASTFunctionDefinition([('x', '', 'Float'), ('base', '0', 'Float')]))
builtins_scope.add_function('log2',  ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('log10', ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('lgamma',ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('pow',   ASTFunctionDefinition([('x', '', 'Float'), ('y', '', 'Float'), ('z', token_to_str('N', Token.Category.CONSTANT), 'BigInt?')]))
builtins_scope.add_function('sqrt',  ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('acos',  ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('asin',  ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('atan',  ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('atan2', ASTFunctionDefinition([('x', '', 'Float'), ('y', '', 'Float')]))
builtins_scope.add_function('cos',   ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('sin',   ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('tan',   ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('degrees', ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('radians', ASTFunctionDefinition([('x', '', 'Float')]))
builtins_scope.add_function('dot',   ASTFunctionDefinition([('v1', '', ''), ('v2', '', '')]))
builtins_scope.add_function('cross', ASTFunctionDefinition([('v1', '', ''), ('v2', '', '')]))
builtins_scope.add_function('perp',      ASTFunctionDefinition([('v', '', '')]))
builtins_scope.add_function('sqlen',     ASTFunctionDefinition([('v', '', '')]))
builtins_scope.add_function('length',    ASTFunctionDefinition([('v', '', '')]))
builtins_scope.add_function('distance',  ASTFunctionDefinition([('p0', '', ''), ('p1', '', '')]))
builtins_scope.add_function('normalize', ASTFunctionDefinition([('v', '', '')]))
builtins_scope.add_function('conjugate', ASTFunctionDefinition([('c', '', '')]))
builtins_scope.add_function('format_float', ASTFunctionDefinition([('x', '', 'Float'), ('precision', '', 'Int')]))
builtins_scope.add_function('format_float_exp', ASTFunctionDefinition([('x', '', 'Float'), ('precision', '', 'Int'), ('width', '0', 'Int')]))
builtins_scope.add_function('commatize', ASTFunctionDefinition([('number', '', 'Int'), ('sep', token_to_str('‘,’'), 'String'), ('step', '3', 'Int')]))
builtins_scope.add_function('gconvfmt', ASTFunctionDefinition([('number', '', 'Float')]))
builtins_scope.add_function('ValueError', ASTFunctionDefinition([('s', '', 'String')]))
builtins_scope.add_function('IndexError', ASTFunctionDefinition([('index', '', 'Int')]))
builtins_scope.add_function('NotImplementedError', ASTFunctionDefinition([]))
builtins_scope.add_function('RuntimeError', ASTFunctionDefinition([('message', '', 'String')]))
builtins_scope.add_function('AssertionError', ASTFunctionDefinition([('message', '', 'String')]))

def add_builtin_global_var(var_name, var_type, var_type_args = []):
    var = ASTVariableDeclaration()
    var.vars = [var_name]
    var.type = var_type
    var.type_args = var_type_args
    builtins_scope.add_name(var_name, var)
add_builtin_global_var('argv', 'Array', ['String'])
add_builtin_global_var('stdin', 'File')
add_builtin_global_var('stdout', 'File')
add_builtin_global_var('stderr', 'File')

builtins_scope.add_name('Char', ASTTypeDefinition([ASTFunctionDefinition([('code', '', 'Int')])]))
char_scope = Scope(None)
char_scope.add_name('is_digit', ASTFunctionDefinition([]))
char_scope.add_name('is_surrogate', ASTFunctionDefinition([]))
builtins_scope.ids['Char'].ast_nodes[0].scope = char_scope
builtins_scope.ids['Char'].ast_nodes[0].constructors[0].first_named_only_argument = 0

builtins_scope.add_name('File', ASTTypeDefinition([ASTFunctionDefinition([('name', '', 'String'), ('encoding', token_to_str('‘utf-8’'), 'String')])]))
file_scope = Scope(None)
file_scope.add_name('at_eof', ASTFunctionDefinition([]))
file_scope.add_name('read_bytes', ASTFunctionDefinition([('size', token_to_str('N', Token.Category.CONSTANT), 'Int?')]))
file_scope.add_name('write_bytes', ASTFunctionDefinition([('bytes', '', '[Byte]')]))
file_scope.add_name('read', ASTFunctionDefinition([('size', token_to_str('N', Token.Category.CONSTANT), 'Int?')]))
file_scope.add_name('write', ASTFunctionDefinition([('s', '', 'String')]))
file_scope.add_name('read_lines', ASTFunctionDefinition([('keep_newline', token_to_str('0B', Token.Category.CONSTANT), 'Bool')]))
file_scope.add_name('read_line',  ASTFunctionDefinition([('keep_newline', token_to_str('0B', Token.Category.CONSTANT), 'Bool')]))
file_scope.add_name('seek', ASTFunctionDefinition([('offset', '', 'Int'), ('whence', '0', 'Int')]))
file_scope.add_name('flush', ASTFunctionDefinition([]))
file_scope.add_name('close', ASTFunctionDefinition([]))
file_scope.add_name('is_associated_with_console', ASTFunctionDefinition([]))
builtins_scope.ids['File'].ast_nodes[0].scope = file_scope

builtins_scope.add_name('FileWr', ASTTypeDefinition([ASTFunctionDefinition([('name', '', 'String'), ('encoding', token_to_str('‘utf-8’'), 'String'), ('append', token_to_str('0B', Token.Category.CONSTANT), 'Bool')])]))
file_scope = Scope(None)
file_scope.add_name('write_bytes', ASTFunctionDefinition([('bytes', '', '[Byte]')]))
file_scope.add_name('write', ASTFunctionDefinition([('s', '', 'String')]))
file_scope.add_name('flush', ASTFunctionDefinition([]))
file_scope.add_name('close', ASTFunctionDefinition([]))
builtins_scope.ids['FileWr'].ast_nodes[0].scope = file_scope

for type_ in cpp_type_from_11l:
    builtins_scope.add_name(type_, ASTTypeDefinition([ASTFunctionDefinition([('object', token_to_str('‘’'), '')])]))

f = ASTFunctionDefinition([('x', '', ''), ('radix', '10', 'Int')])
f.first_named_only_argument = 1
builtins_scope.ids['Int'].ast_nodes[0] = ASTTypeDefinition([f])
builtins_scope.ids['BigInt'].ast_nodes[0] = ASTTypeDefinition([f])

f = ASTFunctionDefinition([('real', '', 'Float'), ('imag', '0', 'Float')])
builtins_scope.ids['Complex'].ast_nodes[0] = ASTTypeDefinition([f])

f = ASTFunctionDefinition([('numerator', '', 'BigInt'), ('denominator', '1', 'BigInt')])
builtins_scope.ids['Fraction'].ast_nodes[0] = ASTTypeDefinition([f])

string_scope = Scope(None)
str_last_member_var_decl = ASTVariableDeclaration()
str_last_member_var_decl.type = 'Char'
string_scope.add_name('last', str_last_member_var_decl)
string_scope.add_name('starts_with', ASTFunctionDefinition([('prefix', '', 'String')]))
string_scope.add_name('ends_with', ASTFunctionDefinition([('suffix', '', 'String')]))
string_scope.add_name('split', ASTFunctionDefinition([('delim', '', 'String'), ('limit', token_to_str('N', Token.Category.CONSTANT), 'Int?'), ('group_delimiters', token_to_str('0B', Token.Category.CONSTANT), 'Bool')]))
string_scope.add_name('split_py', ASTFunctionDefinition([]))
string_scope.add_name('rtrim', ASTFunctionDefinition([('s', '', 'String'), ('limit', token_to_str('N', Token.Category.CONSTANT), 'Int?')]))
string_scope.add_name('ltrim', ASTFunctionDefinition([('s', '', 'String'), ('limit', token_to_str('N', Token.Category.CONSTANT), 'Int?')]))
string_scope.add_name('trim', ASTFunctionDefinition([('s', '', 'String')]))
string_scope.add_name('find', ASTFunctionDefinition([('s', '', 'String'), ('start', '0', 'Int')]))
string_scope.add_name('rfind', ASTFunctionDefinition([('s', '', 'String'), ('start', '0', 'Int')]))
string_scope.add_name('findi', ASTFunctionDefinition([('s', '', 'String'), ('start', '0', 'Int')]))
string_scope.add_name('rfindi', ASTFunctionDefinition([('s', '', 'String'), ('start', '0', 'Int'), ('end', token_to_str('N', Token.Category.CONSTANT), 'Int?')]))
string_scope.add_name('index', ASTFunctionDefinition([('s', '', 'String'), ('start', '0', 'Int')]))
string_scope.add_name('count', ASTFunctionDefinition([('s', '', 'String')]))
string_scope.add_name('replace', ASTFunctionDefinition([('old', '', 'String'), ('new', '', 'String')]))
string_scope.add_name('lowercase', ASTFunctionDefinition([]))
string_scope.add_name('uppercase', ASTFunctionDefinition([]))
string_scope.add_name('capitalize', ASTFunctionDefinition([]))
string_scope.add_name('zfill', ASTFunctionDefinition([('width', '', 'Int')]))
string_scope.add_name('center', ASTFunctionDefinition([('width', '', 'Int'), ('fillchar', token_to_str('‘ ’'), 'Char')]))
string_scope.add_name('ljust',  ASTFunctionDefinition([('width', '', 'Int'), ('fillchar', token_to_str('‘ ’'), 'Char')]))
string_scope.add_name('rjust',  ASTFunctionDefinition([('width', '', 'Int'), ('fillchar', token_to_str('‘ ’'), 'Char')]))
string_scope.add_name('format', ASTFunctionDefinition([('arg', token_to_str('N', Token.Category.CONSTANT), '')] * 32))
string_scope.add_name('map', ASTFunctionDefinition([('function', '', '(Char -> T)')]))
string_scope.add_name('filter', ASTFunctionDefinition([('function', '', '(Char -> Bool)')]))
string_scope.add_name('reduce', ASTFunctionDefinition([('initial', '', 'T'), ('function', '', '((T, Char) -> T)')]))
string_scope.add_name('encode', ASTFunctionDefinition([('encoding', token_to_str('‘utf-8’'), 'String')]))
string_scope.add_name('insert', ASTFunctionDefinition([('i', '', 'Int'), ('s', '', 'String')]))
builtins_scope.ids['String'].ast_nodes[0].scope = string_scope
array_scope = Scope(None)
arr_last_member_var_decl = ASTVariableDeclaration()
arr_last_member_var_decl.type = 'T'
array_scope.add_name('last', arr_last_member_var_decl)
array_scope.add_name('append', ASTFunctionDefinition([('x', '', '')]))
array_scope.add_name('extend', ASTFunctionDefinition([('t', '', '')]))
array_scope.add_name('remove', ASTFunctionDefinition([('x', '', '')]))
array_scope.add_name('count', ASTFunctionDefinition([('x', '', '')]))
array_scope.add_name('index', ASTFunctionDefinition([('x', '', ''), ('start', '0', 'Int'), ('end', '-1', 'Int')]))
array_scope.add_name('find', ASTFunctionDefinition([('x', '', ''), ('start', '0', 'Int')]))
array_scope.add_name('del', ASTFunctionDefinition([('range', '', 'Range')]))
array_scope.add_name('pop', ASTFunctionDefinition([('i', '-1', 'Int')]))
array_scope.add_name('pop_fast', ASTFunctionDefinition([('i', '', 'Int')]))
array_scope.add_name('insert', ASTFunctionDefinition([('i', '', 'Int'), ('x', '', '')]))
array_scope.add_name('reverse', ASTFunctionDefinition([]))
array_scope.add_name('reverse_range', ASTFunctionDefinition([('range', '', 'Range')]))
array_scope.add_name('next_permutation', ASTFunctionDefinition([]))
array_scope.add_name('is_sorted', ASTFunctionDefinition([]))
array_scope.add_name('clear', ASTFunctionDefinition([]))
array_scope.add_name('drop', ASTFunctionDefinition([]))
array_scope.add_name('map', ASTFunctionDefinition([('f', '', '')]))
array_scope.add_name('filter', ASTFunctionDefinition([('f', '', '')]))
array_scope.add_name('join', ASTFunctionDefinition([('sep', '', 'String')]))
array_scope.add_name('sort', ASTFunctionDefinition([('key', token_to_str('N', Token.Category.CONSTANT), ''), ('reverse', token_to_str('0B', Token.Category.CONSTANT), 'Bool')]))
array_scope.add_name('sort_range', ASTFunctionDefinition([('range', '', 'Range'), ('key', token_to_str('N', Token.Category.CONSTANT), ''), ('reverse', token_to_str('0B', Token.Category.CONSTANT), 'Bool')]))
array_scope.add_name('decode', ASTFunctionDefinition([('encoding', token_to_str('‘utf-8’'), 'String')]))
array_scope.add_name('hex', ASTFunctionDefinition([]))
builtins_scope.ids['Array'].ast_nodes[0].scope = array_scope
builtins_scope.ids['Bytes'].ast_nodes[0].scope = array_scope
builtins_scope.ids['ArrayMaxLen'].ast_nodes[0].scope = array_scope
dict_scope = Scope(None)
dict_scope.add_name('update', ASTFunctionDefinition([('other', '', 'Dict')]))
dict_scope.add_name('find', ASTFunctionDefinition([('key', '', '')]))
dict_scope.add_name('get', ASTFunctionDefinition([('key', '', ''), ('default', '', '')]))
dict_scope.add_name('set_default', ASTFunctionDefinition([('key', '', ''), ('default', '', '')]))
dict_scope.add_name('pop', ASTFunctionDefinition([('key', '', '')]))
dict_scope.add_name('clear', ASTFunctionDefinition([]))
dict_scope.add_name('keys', ASTFunctionDefinition([]))
dict_scope.add_name('values', ASTFunctionDefinition([]))
dict_scope.add_name('items', ASTFunctionDefinition([]))
dict_scope.add_name('map', ASTFunctionDefinition([('f', '', '')]))
builtins_scope.ids['Dict'].ast_nodes[0].scope = dict_scope
builtins_scope.ids['DefaultDict'].ast_nodes[0].scope = dict_scope
builtins_scope.ids['Counter'].ast_nodes[0].scope = dict_scope
set_scope = Scope(None)
set_scope.add_name('update', ASTFunctionDefinition([('other', '', 'Set')]))
set_scope.add_name('intersection', ASTFunctionDefinition([('other', '', 'Set')]))
set_scope.add_name('difference', ASTFunctionDefinition([('other', '', 'Set')]))
set_scope.add_name('symmetric_difference', ASTFunctionDefinition([('other', '', 'Set')]))
set_scope.add_name('is_subset', ASTFunctionDefinition([('other', '', 'Set')]))
set_scope.add_name('add', ASTFunctionDefinition([('elem', '', '')]))
set_scope.add_name('pop', ASTFunctionDefinition([]))
set_scope.add_name('remove', ASTFunctionDefinition([('elem', '', '')]))
set_scope.add_name('discard', ASTFunctionDefinition([('elem', '', '')]))
set_scope.add_name('clear', ASTFunctionDefinition([]))
set_scope.add_name('map', ASTFunctionDefinition([('f', '', '')]))
set_scope.add_name('filter', ASTFunctionDefinition([('f', '', '')]))
builtins_scope.ids['Set'].ast_nodes[0].scope = set_scope
deque_scope = Scope(None)
deque_scope.add_name('append', ASTFunctionDefinition([('x', '', '')]))
deque_scope.add_name('append_left', ASTFunctionDefinition([('x', '', '')]))
deque_scope.add_name('pop_left', ASTFunctionDefinition([]))
deque_scope.add_name('pop', ASTFunctionDefinition([]))
builtins_scope.ids['Deque'].ast_nodes[0].scope = deque_scope

module_scope = Scope(None)
builtin_modules['math'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('get_temp_dir', ASTFunctionDefinition([]))
module_scope.add_function('list_dir', ASTFunctionDefinition([('path', token_to_str('‘.’'), 'String')]))
module_scope.add_function('walk_dir', ASTFunctionDefinition([('path', token_to_str('‘.’'), 'String'), ('dir_filter', token_to_str('N', Token.Category.CONSTANT), '(String -> Bool)?'), ('files_only', token_to_str('1B', Token.Category.CONSTANT), 'Bool')]))
module_scope.add_function('is_dir', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('is_file', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('is_symlink', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('file_size', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('resize_file', ASTFunctionDefinition([('path', '', 'String'), ('new_size', '', 'Int')]))
module_scope.add_function('create_dir', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('create_dirs', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('remove_file', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('remove_dir', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('remove_all', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('rename', ASTFunctionDefinition([('old_path', '', 'String'), ('new_path', '', 'String')]))
builtin_modules['fs'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('join', ASTFunctionDefinition([('path1', '', 'String'), ('path2', '', 'String'), ('path3', token_to_str('N', Token.Category.CONSTANT), 'String')]))
module_scope.add_function('base_name', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('dir_name', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('absolute', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('relative', ASTFunctionDefinition([('path', '', 'String'), ('base', '', 'String')]))
module_scope.add_function('canonical', ASTFunctionDefinition([('path', '', 'String')]))
module_scope.add_function('split_ext', ASTFunctionDefinition([('path', '', 'String')]))
builtin_modules['fs::path'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('read', ASTFunctionDefinition([('file_or_file_name', '', ''), ('encoding', token_to_str('‘utf-8’'), 'String'), ('delimiter', token_to_str('‘,’'), 'String'), ('header', token_to_str('N', Token.Category.CONSTANT), '[String]?', '&'), ('skip_first_row', token_to_str('1B', Token.Category.CONSTANT), 'Bool')]))
module_scope.add_name('Writer', ASTTypeDefinition([ASTFunctionDefinition([('file_or_file_name', '', ''), ('encoding', token_to_str('‘utf-8’'), 'String'), ('delimiter', token_to_str('‘,’'), 'String')])]))
#csv_writer_scope = Scope(None)
#module_scope.ids['Writer'].ast_nodes[0].scope = csv_writer_scope
builtin_modules['csv'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('', ASTFunctionDefinition([('command', '', 'String')]))
module_scope.add_function('getenv', ASTFunctionDefinition([('name', '', 'String'), ('default', token_to_str('‘’'), 'String')]))
module_scope.add_function('setenv', ASTFunctionDefinition([('name', '', 'String'), ('value', '', 'String')]))
builtin_modules['os'] = Module(module_scope)
builtins_scope.add_name('Time', ASTTypeDefinition([ASTFunctionDefinition([('year', '0', 'Int'), ('month', '1', 'Int'), ('day', '1', 'Int'), ('hour', '0', 'Int'), ('minute', '0', 'Int'), ('second', '0', 'Float')])]))
time_scope = Scope(None)
time_scope.add_name('unix_time', ASTFunctionDefinition([]))
time_scope.add_name('strftime', ASTFunctionDefinition([('format', '', 'String')]))
time_scope.add_name('format', ASTFunctionDefinition([('format', '', 'String')]))
builtins_scope.ids['Time'].ast_nodes[0].scope = time_scope
f = ASTFunctionDefinition([('days', '0', 'Float'), ('hours', '0', 'Float'), ('minutes', '0', 'Float'), ('seconds', '0', 'Float'), ('milliseconds', '0', 'Float'), ('microseconds', '0', 'Float'), ('weeks', '0', 'Float')])
f.first_named_only_argument = 0
builtins_scope.add_name('TimeDelta', ASTTypeDefinition([f]))
time_delta_scope = Scope(None)
time_delta_scope.add_name('days', ASTFunctionDefinition([]))
builtins_scope.ids['TimeDelta'].ast_nodes[0].scope = time_delta_scope
module_scope = Scope(None)
module_scope.add_function('perf_counter', ASTFunctionDefinition([]))
module_scope.add_function('today', ASTFunctionDefinition([]))
module_scope.add_function('strptime', ASTFunctionDefinition([('datetime_string', '', 'String'), ('format', '', 'String')]))
builtin_modules['time'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('', ASTFunctionDefinition([('pattern', '', 'String')]))
builtin_modules['re'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('', ASTFunctionDefinition([('stop', '1', 'Float')]))
module_scope.add_function('seed', ASTFunctionDefinition([('s', '', 'Int')]))
module_scope.add_function('shuffle', ASTFunctionDefinition([('container', '', '', '&')]))
module_scope.add_function('choice', ASTFunctionDefinition([('container', '', '')]))
module_scope.add_function('sample', ASTFunctionDefinition([('population', '', ''), ('k', '', 'Int')]))
builtin_modules['random'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('push',    ASTFunctionDefinition([('array', '', '', '&'), ('item', '', '')]))
module_scope.add_function('pop',     ASTFunctionDefinition([('array', '', '', '&')]))
module_scope.add_function('heapify', ASTFunctionDefinition([('array', '', '', '&')]))
builtin_modules['minheap'] = Module(module_scope)
builtin_modules['maxheap'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('left',  ASTFunctionDefinition([('a', '', ''), ('x', '', ''), ('lo', '0', 'Int'), ('hi', token_to_str('N', Token.Category.CONSTANT), 'Int?')]))
module_scope.add_function('right', ASTFunctionDefinition([('a', '', ''), ('x', '', ''), ('lo', '0', 'Int'), ('hi', token_to_str('N', Token.Category.CONSTANT), 'Int?')]))
builtin_modules['bisect'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('to_object', ASTFunctionDefinition([('json_str', '', 'String'), ('obj', '', '', '&')]))
module_scope.add_function('from_object', ASTFunctionDefinition([('obj', '', ''), ('indent', '4', '')]))
builtin_modules['json'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('to_object', ASTFunctionDefinition([('eldf_str', '', 'String'), ('obj', '', '', '&')]))
module_scope.add_function('from_object', ASTFunctionDefinition([('obj', '', ''), ('indent', '4', 'Int')]))
module_scope.add_function('from_json', ASTFunctionDefinition([('json_str', '', 'String')]))
module_scope.add_function('to_json', ASTFunctionDefinition([('eldf_str', '', 'String')]))
module_scope.add_function('reparse', ASTFunctionDefinition([('eldf_str', '', 'String')]))
module_scope.add_function('test_parse', ASTFunctionDefinition([('eldf_str', '', 'String')]))
builtin_modules['eldf'] = Module(module_scope)
builtin_modules['term'] = Module(Scope(None))
module_scope = Scope(None)
module_scope.add_function('length', ASTFunctionDefinition([('x', '', 'Int')]))
module_scope.add_function('popcount', ASTFunctionDefinition([('x', '', 'Int')]))
builtin_modules['bits'] = Module(module_scope)
module_scope = Scope(None)
module_scope.add_function('malloc', ASTFunctionDefinition([('size', '', 'Size')]))
module_scope.add_function('free',   ASTFunctionDefinition([('ptr', '', 'Ptr')]))
module_scope.add_function('memset', ASTFunctionDefinition([('dest', '', 'Ptr'), ('ch', '', 'Int'), ('count', '', 'Size')]))
builtin_modules['c'] = Module(module_scope)

def parse_and_to_str(tokens_, source_, file_name_, importing_module_ = False, append_main = False, suppress_error_please_wrap_in_copy = False, to_debug_str = False, used_builtin_modules = None, reset_scope = True): # option suppress_error_please_wrap_in_copy is needed to simplify conversion of large Python source into C++
    if len(tokens_) == 0: return ASTProgram().to_str()
    global tokens, source, tokeni, token, break_label_index, scope, global_scope, tokensn, file_name, importing_module, modules, used_builtin_modules_set
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
    if reset_scope:
        scope = Scope(None)
        if not importing_module_:
            global_scope = scope
        scope.parent = builtins_scope
    file_name = file_name_
    importing_module = importing_module_
    prev_modules = modules
    modules = {}
    if used_builtin_modules is not None:
        used_builtin_modules_set = used_builtin_modules
    next_token()

    if to_debug_str:
        return expression().to_debug_str()

    p = ASTProgram()
    parse_internal(p)

    def transformations(node):
        if type(node) == ASTExpression and node.expression.symbol.id == '=' and node.expression.children[1].function_call:
            c1 = node.expression.children[1]
            c1c0 = c1.children[0]
            if c1c0.symbol.id == '.' and len(c1c0.children) == 2 and c1c0.children[1].token_str() == 'read_line': # transform `line = f.read_line()` into `f.read_line(line)`
                c1.children.insert(1, None)
                c1.children.insert(2, node.expression.children[0])
                node.expression.children[0].parent = c1
                node.expression = c1
                node.expression.parent = None
                node.expression.ast_parent = node

        node.walk_children(transformations)
    transformations(p)

    if len(modules):
        p.beginning_extra = "\n".join(map(lambda m: 'namespace ' + m.replace('::', ' { namespace ') + " {\n#include \"" + m.replace('::', '/') + ".hpp\"\n}" + '}'*m.count('::'), modules)) + "\n\n"

    found_reference_to_argv = False
    def traverse(node):
        if type(node) == ASTLoopBreak:
            node.set_break_label_needed()

        def f(e : SymbolNode):
            if len(e.children) == 1 and e.symbol.id == ':' and e.children[0].token_str() == 'argv':
                nonlocal found_reference_to_argv
                found_reference_to_argv = True
                return
            for child in e.children:
                if child is not None:
                    f(child)

        node.walk_expressions(f)
        node.walk_children(traverse)
    traverse(p)

    if found_reference_to_argv:
        if type(p.children[-1]) != ASTMain:
            raise Error("`sys.argv`->`:argv` can be used only after `if __name__ == '__main__':`->`:start:`", tokens[-1])
        p.children[-1].found_reference_to_argv = True
        p.beginning_extra += "Array<String> argv;\n\n"

    s = p.to_str() # call `to_str()` moved here [from outside] because it accesses global variables `source` (via `token.value(source)`) and `tokens` (via `tokens[ti]`)

    if append_main and type(p.children[-1]) != ASTMain:
        s += "\nint main()\n{\n}\n"

    tokens    = prev_tokens
    source    = prev_source
    tokeni    = prev_tokeni
    token     = prev_token
#   scope     = prev_scope
    tokensn   = prev_tokensn
    file_name = prev_file_name
    importing_module = prev_importing_module
    break_label_index = prev_break_label_index
    modules = prev_modules
    if used_builtin_modules is not None:
        used_builtin_modules_set = None

    return s

def print_debug_str(source_):
    print(parse_and_to_str(tokenizer.tokenize(source_), source_, '', to_debug_str = True))
