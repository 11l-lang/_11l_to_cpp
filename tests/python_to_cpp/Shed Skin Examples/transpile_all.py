import os

for fname in os.listdir('.'):
    if fname[0].isdigit() and (fname.endswith('.py') or (fname.endswith('.11l') and '.2.' in fname)):
        if not os.path.isfile(os.path.splitext(fname)[0] + '.cpp'):
            print(fname + ' - skipped')
        else:
            print(fname)
            os.system('python ../../../../11l.py ' + fname + ' -t')
