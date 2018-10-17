import sys, os, tokenizer

if len(sys.argv) < 2:
    print('Usage: python keyletters_to_keywords.py <in-file> [--reverse]')
    exit(1)

reverse = '--reverse' in sys.argv
expected_extention = '.11kw' if reverse else '.11l'
if not sys.argv[1].endswith(expected_extention):
    sys.exit('Input file should have extension `' + expected_extention + '`')

source = open(sys.argv[1], encoding='utf-8-sig').read()
outf   = open(os.path.splitext(sys.argv[1])[0] + ('.11l' if reverse else '.11kw'), 'w', encoding='utf8', newline = "\n")
writepos = 0

try:
    for token in tokenizer.tokenize(source):
        if (token.value(source) in tokenizer.keywords
                or token.category == tokenizer.Token.Category.KEYWORD # for composite keywords (e.g. `L.break`)
                or token.value(source).split('.')[0] in tokenizer.keywords): # for `L.index`
            dot_pos = token.value(source).find('.') # \
            if dot_pos == -1:                       # A dot_pos = token.value(source). {.find(‘.’) ? .len}
                dot_pos = len(token.value(source))  # /
            outf.write(source[writepos:token.start])
            outf.write(tokenizer.keywords[tokenizer.keywords.index(token.value(source)[:dot_pos]) % 11 + 11*2*(not reverse)] + token.value(source)[dot_pos:])
        else:
            outf.write(source[writepos:token.end])
        writepos = token.end

    outf.write(source[writepos:])

except tokenizer.Error as e:
    sys.stderr.write(e.message + ' at char ' + str(e.pos) + "\n")
    sys.exit(-1)
