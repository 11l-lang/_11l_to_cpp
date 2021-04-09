import sys, platform, os

enopt = False

total = 0
ok = 0

for fname in os.listdir('.'):
    if fname.endswith('.cpp'):
        print(fname, end = '', flush = True)
        if sys.platform == 'win32':
            was_break = False
            for version in ['2019', '2017']:
                for edition in ['BuildTools', 'Community', 'Enterprise', 'Professional']:
                    vcvarsall = 'C:\\Program Files' + ' (x86)'*platform.machine().endswith('64') + '\\Microsoft Visual Studio\\' + version + '\\' + edition + R'\VC\Auxiliary\Build\vcvarsall.bat'
                    if os.path.isfile(vcvarsall):
                        was_break = True
                        #print('Using ' + version + '\\' + edition)
                        break # ^L.break
                if was_break:
                    break
            if not was_break:
                sys.exit('''Unable to find vcvarsall.bat!
If you do not have Visual Studio 2017 or 2019 installed please install it or Build Tools for Visual Studio from here[https://visualstudio.microsoft.com/downloads/].''')

            stderr_fname = 'output/' + fname + '.txt'
            r = os.system('"' + vcvarsall + '" ' + ('x64' if platform.machine().endswith('64') else 'x86') + ' > nul && cl.exe /std:c++17 /MT /EHsc /nologo /we4239 ' + '/O2 '*enopt + fname + ' > ' + stderr_fname)
            if r == 0:
                ok += 1
                os.remove(stderr_fname)
            else:
                print(' - error', end = '')
            print()
            total += 1
        else:
            sys.exit('Only win32 platform is supported so far!')

print(str(ok) + '/' + str(total) + ' files are OK')
