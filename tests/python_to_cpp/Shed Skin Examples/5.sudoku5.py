"""
Author: Ali Assaf <ali.assaf.mail@gmail.com>
Copyright: (C) 2010 Ali Assaf
License: GNU General Public License <http://www.gnu.org/licenses/>
"""

from itertools import product

def exact_cover(x1, y):
    x = dict((j, set()) for j in x1)
    for i, row in y.items():
        for j in row:
            x[j].add(i)
    return (x, y)

def select(x, y, r):
    cols : List[Set[Tuple[int, int, int]]] = []
    for j in y[r]:
        for i in x[j]:
            for k in y[i]:
                if k != j:
                    x[k].remove(i)
        cols.append(x.pop(j))
    return cols

def solve(x, y, solution):
    if not x:
        yield list(solution)
    else:
        #c = min(x, key=lambda c: len(x[c])) # shedskin doesn't support closures!
        c = min([(len(x[c]), c) for c in x])[1]
        for r in list(x[c]):
            solution.append(r)
            cols = select(x, y, r)
            for solution in solve(x, y, solution):
                yield solution
            deselect(x, y, r, cols)
            solution.pop()

def solve_sudoku(rr, cc, grid):
    """An efficient Sudoku solver using Algorithm X."""
    nn = rr * cc
    x1 = ([("rc", rc) for rc in product(range(nn), range(nn))] +
          [("rn", rn) for rn in product(range(nn), range(1, nn + 1))] +
          [("cn", cn) for cn in product(range(nn), range(1, nn + 1))] +
          [("bn", bn) for bn in product(range(nn), range(1, nn + 1))])
    y : Dict[int] = {}
    for r, c, n in product(range(nn), range(nn), range(1, nn + 1)):
        b = (r // rr) * rr + (c // cc) # Box number
        y[(r, c, n)] = [
            ("rc", (r, c)),
            ("rn", (r, n)),
            ("cn", (c, n)),
            ("bn", (b, n))]
    x : Dict[int]
    (x, y) = exact_cover(x1, y)
    for i, row in enumerate(grid):
        for j, n in enumerate(row):
            if n:
                select(x, y, (i, j, n))
    g : List[List[List[int]]] = []
    for solution in solve(x, y, []):
        for r, c, n in solution:
            grid[r][c] = n
        g.append(grid)
    return g

def deselect(x, y, r, cols):
    for j in reversed(y[r]):
        x[j] = cols.pop()
        for i in x[j]:
            for k in y[i]:
                if k != j:
                    x[k].add(i)

if __name__ == "__main__":
    grid = [
        [5, 3, 0, 0, 7, 0, 0, 0, 0],
        [6, 0, 0, 1, 9, 5, 0, 0, 0],
        [0, 9, 8, 0, 0, 0, 0, 6, 0],
        [8, 0, 0, 0, 6, 0, 0, 0, 3],
        [4, 0, 0, 8, 0, 3, 0, 0, 1],
        [7, 0, 0, 0, 2, 0, 0, 0, 6],
        [0, 6, 0, 0, 0, 0, 2, 8, 0],
        [0, 0, 0, 4, 1, 9, 0, 0, 5],
        [0, 0, 0, 0, 8, 0, 0, 7, 9],
    ]
    for solution in solve_sudoku(3, 3, grid):
        print("\n".join(str(s) for s in solution))
