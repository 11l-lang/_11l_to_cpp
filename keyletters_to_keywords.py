import sys, os, tokenizer

if len(sys.argv) < 2:
    print('Usage: python keyletters_to_keywords.py <in-file>')
    exit(1)

source = open(sys.argv[1], encoding='utf-8-sig').read()
outf   = open(os.path.splitext(sys.argv[1])[0] + '.11kw', 'w', encoding='utf8')
writepos = 0

try:
    for token in tokenizer.tokenize(source):
        if token.value(source) in tokenizer.keywords \
                or token.category == tokenizer.Token.Category.KEYWORD: # for composite keywords (e.g. `L.break`)
            dot_pos = token.value(source).find('.') # \
            if dot_pos == -1:                       # A dot_pos = token.value(source). {.find(‘.’) ? .len}
                dot_pos = len(token.value(source))  # /
            outf.write(source[writepos:token.start])
            outf.write(tokenizer.keywords[tokenizer.keywords.index(token.value(source)[:dot_pos]) % 11 + 11*2] + token.value(source)[dot_pos:])
        else:
            outf.write(source[writepos:token.end])
        writepos = token.end

    outf.write(source[writepos:])

except tokenizer.Error as e:
    sys.stderr.write(e.message + ' at char ' + str(e.pos) + "\n")
    sys.exit(-1)
