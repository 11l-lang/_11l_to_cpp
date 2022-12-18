import os, subprocess, time

results = []

for fname in os.listdir():
    if fname.endswith('.txt') and len(fname) == 6:
        for problem in open(fname, encoding = 'utf-8').read().split("\n\n\n"):
            if problem.startswith('---'):
                continue

            in_python, in_11l, in_cpp = problem.split("===\n")
            open('problem.py', 'w').write(in_python)

            min_time_py = float('inf')
            total_time = 0.0
            while True:
                start_time = time.perf_counter()
                ans_py = subprocess.check_output(['python', 'problem.py'], encoding = 'utf-8').rstrip()
                elapsed = time.perf_counter() - start_time
                min_time_py = min(min_time_py, elapsed)
                total_time += elapsed
                if total_time >= 1.0:
                    break

            assert(in_python[:2] == '# ')
            problem_id = int(in_python[2:].split("\n", 1)[0].split(' ', 1)[0])
            if os.system(r'..\..\..\..\11l problem.py') != 0:
                print('Compilation error in problem', problem_id)
                continue

            min_time_11l = float('inf')
            total_time = 0.0
            while True:
                start_time = time.perf_counter()
                ans_11l = subprocess.check_output(['problem'], encoding = 'utf-8').rstrip()
                elapsed = time.perf_counter() - start_time
                min_time_11l = min(min_time_11l, elapsed)
                total_time += elapsed
                if total_time >= 1.0:
                    break

            if ans_11l != ans_py:
                print('Wrong result in problem', problem_id)

            results.append((problem_id, min_time_py / min_time_11l))

open('results.txt', 'w', newline = "\n").write("\n".join(f'{problem_id}: {speed_factor}' for problem_id, speed_factor in sorted(results, key = lambda r: r[1], reverse = True)))

os.remove('problem.py')
os.remove('problem.11l')
os.remove('problem.cpp')
os.remove('problem.obj')
os.remove('problem.exe')
