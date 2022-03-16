import os, tokenizer, parse, sys
sys.path.insert(0, '..') # `append(..)` here works incorrectly if 11l package is installed
import python_to_11l.tokenizer as py_tokenizer, python_to_11l.parse as py_parser

for fname in os.listdir('tests/parser'):
    if fname != 'errors.txt':
        tests = []
        full_fname = 'tests/parser/' + fname
        for test in open(full_fname, encoding = 'utf-8').read().split("\n\n\n"):
            if test.startswith('---'):
                tests.append(test)
                continue
            (in_11l, expected_cpp) = test.split("===\n")
            in_cpp = parse.parse_and_to_str(tokenizer.tokenize(in_11l), in_11l, full_fname)
            assert(in_cpp[-1] == "\n")
            tests.append(in_11l + "===\n" + in_cpp[:-1])
        open(full_fname, 'w', encoding = 'utf-8', newline = "\n").write("\n\n\n".join(tests))
        print(full_fname)

for dir in ['tests/python_to_cpp',
            'tests/python_to_cpp/Rosetta Code',
            'tests/python_to_cpp/Project Euler']:
    for fname in os.listdir(dir):
        assert(fname[0] != '-')
        if not fname.endswith('.txt'): # skip .py, .11l and .hpp files
            continue
        if dir.endswith('Rosetta Code') and len(fname) != 5: # process only single letter files in Rosetta Code subdirectory
            continue
        if dir.endswith('Project Euler') and len(fname) != 6: # process only two letter files in Project Euler subdirectory
            continue

        full_fname = dir + '/' + fname
        tests = []
        for test in open(full_fname, encoding = 'utf-8').read().split("\n\n\n"):
            if test.startswith('---'):
                tests.append(test)
                continue
            (in_python, expected_11l, expected_cpp) = test.split("===\n")
            in_11l = py_parser.parse_and_to_str(py_tokenizer.tokenize(in_python), in_python, full_fname)
            assert(in_11l[-1] == "\n")
            in_cpp = parse.parse_and_to_str(tokenizer.tokenize(in_11l), in_11l, full_fname)
            assert(in_cpp[-1] == "\n")
            tests.append(in_python + "===\n" + in_11l + "===\n" + in_cpp[:-1])
        open(full_fname, 'w', encoding = 'utf-8', newline = "\n").write("\n\n\n".join(tests))
        print(full_fname)
