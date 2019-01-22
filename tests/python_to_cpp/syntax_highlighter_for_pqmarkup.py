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
    comments : List[Tuple[int, int]] = []

    if lang == 'Python':
        for token in python_to_11l.tokenizer.tokenize(source, comments = comments):
            print(token.to_str(source))
        print(comments)
    else:
        assert(lang == '11l')

    return source
