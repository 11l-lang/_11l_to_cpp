import os, tokenizer, re

tokenizer_tests = []
for root, dirs, files in os.walk('tests/tokenizer'):
    for name in files:
        tokenizer_tests += open(os.path.join(root, name), encoding="utf8").read().split("\n\n\n")

for n, test in enumerate(tokenizer_tests):
    if test.startswith('---'):
        continue
    test_source = ""
    error = None
    scopes = []
    ellipsises = []
    last_line_len = 0
    def process_line(line):
        global test_source, last_line_len, error, scopes, ellipsises
        if line.startswith('^') or line.endswith("^"):
            if line.startswith('^'):
                position = re.search(r'[^\t ]', line[1:]).start() + 1
                message = line[position:]
            else:
                position = 0
                message = line[:-1]
            position += len(test_source) - last_line_len - 1
            if message.startswith('Error: '):
                error = (message, position)
            elif message[0] in '{}':
                for ch in message:
                    scopes += [(ch, position)]
            elif message == '…':
                ellipsises += [position]
            return
        test_source += line + "\n"
        last_line_len = len(line)

    line_start = 0
    i = 0
    while i < len(test):
        if test[i] == "\n":
            if test[i-3:i] == '```':
                i = test.find('```', i)
                assert(i != -1)
                i += 3
            process_line(test[line_start:i])
            line_start = i+1
        i += 1
    process_line(test[line_start:i])
#    if n >= 1:
#        break
    # print(test_source)
    # print(error)
    implied_scopes = []
    line_continuations = []
    newline_chars = []
    was_error = False
    try:
        tokenizer.tokenize(test_source, implied_scopes, line_continuations, newline_chars)
        if not (implied_scopes == scopes and line_continuations == ellipsises):
            print("Mismatch at test:\n" + test + "\n\n")
            print("scopes_determinator scopes:", implied_scopes)
            print("expected (in test) scopes: ", scopes)
            print("scopes_determinator joined lines:", line_continuations)
            print("expected (in test) joined lines: ", ellipsises)
            break
    except tokenizer.Error as e:
        was_error = True
        if error and "Error: " + e.message == error[0] and e.pos == error[1]:
            print('OK (Error)')
            continue
        else:
            next_line_pos = test_source.find("\n", e.pos)
            if next_line_pos == -1:
                next_line_pos = len(test_source)
            prev_line_pos = test_source.rfind("\n", 0, e.pos) + 1
            # print('Error: ' + e.message + "\n" + test_source[prev_line_pos:next_line_pos] + "\n" + ' '*(e.pos - prev_line_pos) + '^')
            print('Error: ' + e.message + "\n" + test_source[prev_line_pos:next_line_pos] + "\n" + re.sub(r'[^\t]', ' ',
                                                                                                          test_source[
                                                                                                          prev_line_pos:e.pos]) + '^')
            print("in test:\n" + test)
            break
    if error != None and not was_error:
        print("There should be error in test:\n" + test)
        break
    print('OK')
