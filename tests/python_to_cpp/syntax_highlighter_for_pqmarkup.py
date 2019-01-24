import python_to_11l.tokenizer

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

    return res
