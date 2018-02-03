import scopes_determinator, re

for n, test in enumerate(open("tests.txt", encoding="utf8").read().split("\n\n\n")):
    if test.startswith('---'):
        continue
    test_source = ""
    error = None
    scopes = []
    ellipsises = []
    last_line_len = 0
    for line in test.splitlines():
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
            continue
        test_source += line + "\n"
        last_line_len = len(line)
#    if n >= 1:
#        break
    # print(test_source)
    # print(error)
    implied_scopes = []
    line_continuations = []
    newline_chars = []
    was_error = False
    try:
        scopes_determinator.lex(test_source, implied_scopes, line_continuations, newline_chars)
        if implied_scopes == scopes and line_continuations == ellipsises:
            print('OK')
        else:
            print("Mismatch at test:\n" + test + "\n\n")
            print(implied_scopes)
            print(scopes)
            print(line_continuations)
            print(ellipsises)
            break
    except scopes_determinator.Exception as e:
        was_error = True
        if error and "Error: " + e.message == error[0] and e.pos == error[1]:
            print('OK (Error)')
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
