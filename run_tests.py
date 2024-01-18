import os, tokenizer, parse, re, tempfile, sys, locale
sys.path.insert(0, '..') # `append(..)` here works incorrectly if 11l package is installed
import python_to_11l.tokenizer as py_tokenizer, python_to_11l.parse as py_parser

def kdiff3(str1, str2):
    for envvar in ['ProgramFiles', 'ProgramFiles(x86)', 'ProgramW6432']:
        os.environ["PATH"] += os.pathsep + os.getenv(envvar, '') + r'\KDiff3'
    command = 'kdiff3'
    for file in [('left', str1), ('right', str2)]:
        full_fname = os.path.join(tempfile.gettempdir(), file[0])
        command += ' "' + full_fname + '"'
        open(full_fname, "wt", encoding='utf-8-sig').write(file[1])
    os.system(command)

def p(s):
    sys_encoding = locale.getpreferredencoding(False)
    return s.encode(sys_encoding, errors = 'backslashreplace').decode(sys_encoding)

tokenizer_tests = []
for root, dirs, files in os.walk('tests/tokenizer'):
    for name in files:
        tests = open(os.path.join(root, name), encoding="utf8").read()
        if name == 'comments.txt':
            source, expected_comments = tests.split("===\n")
            comments = []
            tokenizer.tokenize(source, comments = comments)
            comments_str = "\n".join(str(t) for t in comments)
            if comments_str != expected_comments:
                print("Comments mismatch for test:\n" + source + "Comments:\n" + comments_str + "\nExpected comments:\n" + expected_comments)
                exit(1)
            else:
                print("OK (Comments)")
        elif name == 'ascii_raw_string_literals.txt':
            for test in tests.split("\n\n\n"):
                original_11l, expected_corrected_11l = test.split("===\n")
                expected_corrected_11l += "\n"
                tokens = tokenizer.tokenize(original_11l)
                if not tokenizer.needs_source_code_correction(tokens, original_11l):
                    print("`tokenizer.needs_source_code_correction()` returned `False` for test:\n" + original_11l)
                    exit(1)
                corrected_11l = tokenizer.correct_source_code(tokens, original_11l)
                if corrected_11l != expected_corrected_11l:
                    print("Mismatch for test:\n" + original_11l + "Output:\n" + corrected_11l + "\nExpected output:\n" + expected_corrected_11l + "[in file '" + name + "']")
                    kdiff3(corrected_11l, expected_corrected_11l)
                    exit(1)
        else:
            tokenizer_tests += tests.split("\n\n\n")

for n, test in enumerate(tokenizer_tests):
    if test.startswith('---'):
        continue
    test_source = ""
    error = None
    scopes = []
    ellipsises = []
    i = 0
    while i < len(test):
        line_start = i
        while True:
            if test[i] not in (' ', "\t"):
                break
            i += 1
            assert(i < len(test))

        if test[i] == '^':
            message_start = i
            while True:
                i = test.find("\n", i+1)
                if i == -1:
                    i = len(test)
                elif test[i-1] == '\\':
                    continue
                break
            message = test[message_start+1:i].replace("\\\n", "\n")
            position = message_start - line_start + len(test_source) - last_line_len - 1
            if message.startswith('Error: '):
                error = (message, position)
            elif message[0] in '{}':
                for ch in message:
                    scopes += [(ch, position)]
            elif message == '…':
                ellipsises += [position]

        else:
            i = test.find("\n", i)
            if i == -1:
                i = len(test)
            test_source += test[line_start:i] + "\n"
            last_line_len = i - line_start

        if i < len(test):
            assert(test[i] == "\n")
            i += 1
#    if n >= 1:
#        break
    # print(test_source)
    # print(error)
    implied_scopes = []
    line_continuations = []
    was_error = False
    try:
        tokenizer.tokenize(test_source, implied_scopes, line_continuations)
        if not (implied_scopes == scopes and line_continuations == ellipsises):
            print("Mismatch at test:\n" + test + "\n\n")
            print("scopes_determinator scopes:", implied_scopes)
            print("expected (in test) scopes: ", scopes)
            print("scopes_determinator joined lines:", line_continuations)
            print("expected (in test) joined lines: ", ellipsises)
            exit(1)
    except tokenizer.Error as e:
        was_error = True
        if error and 'Error: ' + e.message == error[0] and e.pos == error[1]:
            print('OK (Error)')
            continue
        else:
            kdiff3('Error: ' + e.message, error[0] if error is not None else '')
            next_line_pos = test_source.find("\n", e.pos)
            if next_line_pos == -1:
                next_line_pos = len(test_source)
            prev_line_pos = test_source.rfind("\n", 0, e.pos) + 1
            # print('Error: ' + e.message + "\n" + test_source[prev_line_pos:next_line_pos] + "\n" + ' '*(e.pos - prev_line_pos) + '^')
            print('Error: ' + e.message + "\n" + test_source[prev_line_pos:next_line_pos] + "\n" + re.sub(r'[^\t]', ' ',
                                                                                                          test_source[
                                                                                                          prev_line_pos:e.pos]) + '^')
            print("in test:\n" + test)
            exit(1)
    if error is not None and not was_error:
        print("There should be error in test:\n" + test)
        exit(1)
    print('OK')

for fname in os.listdir('tests/parser'):
    if not fname.endswith('.txt'): # skip .11l and .hpp files
        continue
    full_fname = 'tests/parser/' + fname
    for test in open(full_fname, encoding="utf8").read().split("\n\n\n"):
        if test.startswith('---'):
            continue
        if fname == 'errors.txt':
            try:
                source, error_message = test.split('^Error: ')
                full_source = source.rstrip()
                npos = error_message.find("\n")
                if npos != -1:
                    if error_message[npos:].rstrip() != '':
                        full_source += error_message[npos:]
                    error_message = error_message[:npos]
                parse.parse_and_to_str(tokenizer.tokenize(full_source), full_source, full_fname)
            except parse.Error as e:
                line_start = source.rfind("\n", 0, len(source))
                if e.message == error_message and e.pos == source.rfind("\n", 0, line_start) + len(source) - line_start:
                    print('OK (Error)')
                    #py_parser.file_name = ''
                    parse.file_name = ''
                    continue
                else:
                    kdiff3(e.message, error_message)
                    print('Error at position ' + str(e.pos) + " in test:\n" + test + "\n[in file '" + full_fname + "']")
                    exit(1)
            except Exception as e:
                print("Exception in test:\n" + test + "\n[in file '" + full_fname + "']")
                raise e
            print("There should be error in test:\n" + test)
            exit(1)
        else:
            try:
                in_11l, expected_cpp = test.split("===\n")
                expected_cpp += "\n"
                tokens = tokenizer.tokenize(in_11l)
                for token in tokens:
                    if token.category == token.category.SCOPE_BEGIN and in_11l[token.start] != '{' and (token.end - token.start) % 3 != 0: # }
                        if "r[L.index] = S c\n" not in in_11l and "V tag = S ‘_’\n" not in in_11l and ' = S ' not in in_11l:
                            print("Wrong indentation:\n" + in_11l + "[in file '" + full_fname + "']")
                            exit(1)
                in_cpp = parse.parse_and_to_str(tokens, in_11l, full_fname)
                if in_cpp != expected_cpp:
                    print("Mismatch for test:\n" + in_11l + "Output:\n" + in_cpp + "\nExpected output:\n" + expected_cpp + "[in file '" + full_fname + "']")
                    kdiff3(in_cpp, expected_cpp)
                    exit(1)
            except Exception as e:
                print("Exception in test:\n" + test + "\n[in file '" + full_fname + "']")
                raise e
            print('OK')

for fname in os.listdir('tests/python_to_cpp'):
    full_fname = 'tests/python_to_cpp/' + fname
    if fname[0] == '-':
        continue
    if not fname.endswith('.txt'): # skip .py, .11l and .hpp files
        continue
    for test in open(full_fname, encoding="utf8").read().split("\n\n\n"):
        if test.startswith('---'):
            continue
        try:
            in_python, expected_11l, expected_cpp = test.split("===\n")
            expected_cpp += "\n"
            in_11l = py_parser.parse_and_to_str(py_tokenizer.tokenize(in_python), in_python, full_fname)
            if in_11l != expected_11l:
                print("Mismatch 11l for test:\n" + test + "\n[in file '" + full_fname + "']")
                kdiff3(in_11l, expected_11l)
                exit(1)
            in_cpp = parse.parse_and_to_str(tokenizer.tokenize(expected_11l), expected_11l, full_fname)
            if in_cpp != expected_cpp:
                print("Mismatch C++ for test:\n" + p(test) + "\n[in file '" + full_fname + "']")
                kdiff3(in_cpp, expected_cpp)
                exit(1)
        except Exception as e:
            print("Exception in test:\n" + test + "\n[in file '" + full_fname + "']")
            file_name = py_parser.file_name if type(e) == py_parser.Error else parse.file_name
            if file_name != '':
                print("[in file '" + file_name + "']")
            raise e
        print('OK')

print('All OK')
