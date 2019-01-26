import python_to_11l.tokenizer
import _11l_to_cpp.tokenizer

css = R'''<style>
span.keyword {color: #0000FF; font-weight: bold;}
span.identifier {color: #00009F;}
span.string-literal {color: #800000;}
span.numeric-literal {color: #008000;}
span.constant {color: #008000;}
span.comment {color: #808080;}
</style>'''

cat_to_class_python = {
    python_to_11l.tokenizer.Token.Category.NAME                  : 'identifier',
    python_to_11l.tokenizer.Token.Category.KEYWORD               : 'keyword',
    python_to_11l.tokenizer.Token.Category.CONSTANT              : 'constant',
    python_to_11l.tokenizer.Token.Category.OPERATOR_OR_DELIMITER : '',
    python_to_11l.tokenizer.Token.Category.NUMERIC_LITERAL       : 'numeric-literal',
    python_to_11l.tokenizer.Token.Category.STRING_LITERAL        : 'string-literal',
    python_to_11l.tokenizer.Token.Category.INDENT                : '',
    python_to_11l.tokenizer.Token.Category.DEDENT                : '',
    python_to_11l.tokenizer.Token.Category.STATEMENT_SEPARATOR   : '',
}

cat_to_class_11l = {
    _11l_to_cpp.tokenizer.Token.Category.NAME                : 'identifier',
    _11l_to_cpp.tokenizer.Token.Category.KEYWORD             : 'keyword',
    _11l_to_cpp.tokenizer.Token.Category.CONSTANT            : 'constant',
    _11l_to_cpp.tokenizer.Token.Category.DELIMITER           : '',
    _11l_to_cpp.tokenizer.Token.Category.OPERATOR            : '',
    _11l_to_cpp.tokenizer.Token.Category.NUMERIC_LITERAL     : 'numeric-literal',
    _11l_to_cpp.tokenizer.Token.Category.STRING_LITERAL      : 'string-literal',
    _11l_to_cpp.tokenizer.Token.Category.STRING_CONCATENATOR : '', # why '' and not 'string-literal': because this is rather operator than string literal
    _11l_to_cpp.tokenizer.Token.Category.SCOPE_BEGIN         : '',
    _11l_to_cpp.tokenizer.Token.Category.SCOPE_END           : '',
    _11l_to_cpp.tokenizer.Token.Category.STATEMENT_SEPARATOR : '',
}

def is_lang_supported(lang):
    return lang in ('11l', 'Python')

class Error(Exception):
    message : str
    pos : int
    def __init__(self, message, pos):
        self.message = message
        self.pos = pos

def highlight(lang, source):
    writepos = 0
    comments : List[Tuple[int, int]] = []
    res = ''

    def html_escape(s):
        return s.replace('&', '&amp;').replace('<', '&lt;')

    if lang == 'Python':
        try:
            for token in python_to_11l.tokenizer.tokenize(source, comments = comments) + [python_to_11l.tokenizer.Token(len(source), len(source), python_to_11l.tokenizer.Token.Category.STATEMENT_SEPARATOR)]:
                while len(comments) and comments[0][0] < token.start:
                    res += html_escape(source[writepos:comments[0][0]])
                    writepos = comments[0][1]
                    res += '<span class="comment">' + html_escape(source[comments[0][0]:comments[0][1]]) + '</span>'
                    comments.pop(0)
                res += html_escape(source[writepos:token.start])
                writepos = token.end
                css_class = cat_to_class_python[token.category]
                if css_class != '':
                    res += '<span class="' + css_class + '">' + html_escape(token.value(source)) + '</span>'
                else:
                    res += html_escape(token.value(source))

        except python_to_11l.tokenizer.Error as e:
            raise Error(e.message, e.pos)
    else:
        assert(lang == '11l')
        try:
            for token in _11l_to_cpp.tokenizer.tokenize(source, comments = comments) + [_11l_to_cpp.tokenizer.Token(len(source), len(source), _11l_to_cpp.tokenizer.Token.Category.STATEMENT_SEPARATOR)]:
                while len(comments) and comments[0][0] < token.start:
                    res += html_escape(source[writepos:comments[0][0]])
                    writepos = comments[0][1]
                    res += '<span class="comment">' + html_escape(source[comments[0][0]:comments[0][1]]) + '</span>'
                    comments.pop(0)
                res += html_escape(source[writepos:token.start])
                writepos = token.end

                tokstr = html_escape(token.value(source))
                css_class : str
                if (token.category == _11l_to_cpp.tokenizer.Token.Category.NAME and tokstr in ('V', 'П', 'var', 'перем')) \
                or (token.category == _11l_to_cpp.tokenizer.Token.Category.OPERATOR and tokstr in ('C', 'С', 'in', '!C', '!С', '!in')) \
                or tokstr.split('.')[0] in _11l_to_cpp.tokenizer.keywords:
                    css_class = 'keyword'
                else:
                    css_class = cat_to_class_11l[token.category]

                if css_class != '':
                    res += '<span class="' + css_class + '">' + tokstr + '</span>'
                else:
                    res += tokstr

        except _11l_to_cpp.tokenizer.Error as e:
            raise Error(e.message, e.pos)

    return res
