import sys, os, tokenizer, parse

if len(sys.argv) < 2:
    print('Usage: python russian_to_english.py <in-file>')
    sys.exit(1)

source = open(sys.argv[1], encoding = 'utf-8-sig').read()
(root, ext) = os.path.splitext(sys.argv[1])
outf = open(root + '_en' + ext, 'w', encoding = 'utf-8', newline = "\n")
writepos = 0

additional_russian_names = {'прервать': 'break', 'продолжить': 'continue', 'индекс': 'index', 'ответ': 'answer', 'массив': 'array', 'индексы': 'inds'}
all_russian_names = dict(parse.russian_names, **additional_russian_names) # [https://stackoverflow.com/questions/9819602/union-of-dict-objects-in-python <- google:‘python dict union’]

subkeywords = {
    'Е.часто': 'likely',
    'Е.редко': 'unlikely',
    'Ф.арг': 'args',
    'Ф.деструктор': 'destructor',
    'Ф.виртуал.новая': 'virtual.new',
    'Ф.виртуал.переопр': 'virtual.override',
    'Ф.виртуал.финал': 'virtual.final',
    'Ф.виртуал.абстракт': 'virtual.abstract',
    'Ф.виртуал.опред': 'virtual.assign',
    'Ц.продолжить': 'continue',
    'Ц.прервать': 'break',
    'Ц.удалить_текущий_элемент_и_продолжить': 'remove_current_element_and_continue',
    'Ц.удалить_текущий_элемент_и_прервать': 'remove_current_element_and_break',
    'Ц.при_продолжении': 'on_continue',
    'Ц.при_прерывании': 'on_break',
    'Ц.не_был_прерван': 'was_no_break',
    'Ц.индекс': 'index',
    'Ц.первая_итерация': 'first_iteration',
    'Ц.последняя_итерация': 'last_iteration',
    'Ц.след': 'next',
    'Ц.пред': 'prev',
    'В.выйти': 'break',
    'Т.базовый': 'base',
    'Т.перечисл': 'enum',
    'Т.интерфейс': 'interface',
    'Х.контроль': 'try',
    'Х.перехват': 'catch',
}

try:
    for token in tokenizer.tokenize(source):
        if (token.value(source) in tokenizer.keywords
                or token.category == tokenizer.Token.Category.KEYWORD # for composite keywords (e.g. `L.break`)
                or token.value(source).split('.')[0] in tokenizer.keywords # for `L.index`
                or (token.value(source).startswith('!') and token.value(source)[1:] in tokenizer.keywords)): # for `!С`/`!св`
            dot_pos = token.value(source).find('.') # \
            if dot_pos == -1:                       # V dot_pos = token.value(source). {.find(‘.’) ? .len}
                dot_pos = len(token.value(source))  # /
            em = int(token.value(source)[0] == '!')
            outf.write(source[writepos:token.start])

            ind = tokenizer.keywords.index(token.value(source)[em:dot_pos])
            if (ind // 11) in (0, 2): # this is English keyword — translation is not needed
                outf.write(token.value(source))
            else:
                outf.write('!'*em + tokenizer.keywords[ind - 11])
                if '.' in token.value(source):
                    assert(em == 0)
                    if ind // 11 == 3: ind -= 22 # to one-letter keyword
                    outf.write('.' + subkeywords[tokenizer.keywords[ind] + token.value(source)[dot_pos:]])

        elif token.value(source) in all_russian_names:
            outf.write(source[writepos:token.start])
            outf.write(all_russian_names[token.value(source)])

        else:
            outf.write(source[writepos:token.end])

        writepos = token.end

    outf.write(source[writepos:])

except tokenizer.Error as e:
    sys.stderr.write(e.message + ' at char ' + str(e.pos) + "\n")
    sys.exit(-1)
