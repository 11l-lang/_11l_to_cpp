# (c) Peter Goodspeed
# --- coriolinus@gmail.com
#
# sudoku solver

import math
import time
import sys

from typing import List, Set, Tuple
Byte = int

class bmp:
        v : List[Byte]
        def __init__(self, vals=9*[Byte(1)], n=-1):
                self.v = vals[0:9]
                if n>=0: self.v[n] = not self.v[n]
        def __and__(self, other):
                return bmp([Byte(self.v[i] and other.v[i]) for i in range(9)])
        def cnt(self):
                return len([i for i in self.v if i])

def calc_hash_of_str(s): # uses FNV-1a algorithm
        hash = 2166136261
        for c in s:
                hash = (16777619 * (hash ^ ord(c))) & 0xFFFFFFFF;
        return hash

class BoardRep:
        __fields : List[List[int]]
        def __init__(self, board):
                self.__fields = board.final
        # def fields(self):
        #         return self.__fields
        # def __eq__(self, other):
        #         return self.__fields==other.fields()
        # def __ne__(self, other):
        #         return self.__fields!=other.fields()
        def __hash__(self):
                rep=""
                for i in range(9):
                        for j in range(9):
                                rep += str(self.__fields[i][j])
                return calc_hash_of_str(rep)
        def __lt__(self, other):
            return self.__fields < other.__fields

board_notifyOnCompletion = True               #let the user know when you're done computing a game
board_completeSearch = False                  #search past the first solution

class board:
        final : List[List[int]]
        rows : List[bmp]
        cols : List[bmp]
        cels : List[List[bmp]]

        #statistics
        __turns = 0
        __backtracks = 0
        __starttime = 0.0
        __endtime = 0.0
        __status = 0
        __maxdepth = 0
        __openspaces = 81

        solutions : Set[BoardRep] # this is needed for 11l and to fix mypy error: Need type annotation for 'solutions' (comment this line and uncomment ` -> None:` to see this error)
        examined  : Set[BoardRep] # this is needed for 11l and to fix mypy error: Need type annotation for 'examined'  (comment this line and uncomment ` -> None:` to see this error)

        def __init__(self):# -> None:
                #final numbers: a 9 by 9 grid
                self.final = [9 * [0] for i in range(9)]
                self.rows = 9 * [bmp()]
                self.cols = 9 * [bmp()]
                self.cels = [3 * [bmp()] for i in range(3)]

                #a set of all solved boards discovered so far
                self.solutions = set()
                #a set of all boards examined--should help reduce the amount of search duplication
                self.examined = set()

        def fread(self,filename=''):
                #self.__init__()
                if filename=='':
                        print("filename: ")
                        filename = input()
                f = open(filename, 'r')
                lines = f.readlines()
                for row in range(9):
                        for digit in range(1,10):
                                try:
                                        self.setval(row,lines[row].index(str(digit)),digit)
                                except ValueError:
                                        pass
                #f.close()

        def setval(self, row, col, val):
                #add the number to the grid
                self.final[row][col] = val
                self.__openspaces -= 1

                #remove the number from the potential masks
                mask = bmp(n = val - 1)
                #rows and cols
                self.rows[row] = self.rows[row] & mask
                self.cols[col] = self.cols[col] & mask
                #cels
                cr = self.cell(row)
                cc = self.cell(col)
                self.cels[cr][cc] = self.cels[cr][cc] & mask

        def cell(self, num): # const
                return int(math.ceil((num + 1) / 3.0)) - 1

        def to_str(self): # const
                ret = ""
                for row in range(9):
                        if row == 3 or row == 6: ret += (((3 * "---") + "+") * 3)[:-1] + "\n"
                        for col in range(9):
                                if col == 3 or col == 6: ret += "|"
                                ret += " " + (str(self.final[row][col]) if self.final[row][col] != 0 else " ") + " "
                        ret += "\n"
                return ret

        def solve(self, notify=True, completeSearch=False):
                if self.__status == 0:
                        self.__status = 1
                        self.__starttime = time.time()
                        board_notifyOnCompletion = notify
                        board_completeSearch = completeSearch
                        self.__solve(self, 0)

        def openspaces(self): # const
                return self.__openspaces

        def __solve(self, _board, depth) -> None:
                if BoardRep(_board) not in self.examined:
                        self.examined.add(BoardRep(_board))

                        #check for solution condition:
                        if _board.openspaces() <= 0:
                                self.solutions.add(BoardRep(_board))
                                print('solution:')
                                print(_board.to_str())
                                if depth == 0: self.onexit()
                                if not board_completeSearch:
                                        self.onexit()

                        else:
                                #update the statistics
                                self.__turns += 1
                                if depth > self.__maxdepth: self.__maxdepth = depth

                                #figure out the mincount
                                mincnt : int
                                coords : List[Tuple[int, int]]
                                (mincnt, coords) = _board.findmincounts()
                                if mincnt <= 0:
                                        self.__backtracks += 1
                                        if depth == 0: self.onexit()
                                else:
                                        #coords is a list of tuples of coordinates with equal, minimal counts
                                        # of possible values. Try each of them in turn.
                                        for row, col in coords:
                                                #now we iterate through possible values to put in there
                                                broken = False
                                                for val in [i for i in range(9) if _board.mergemask(row, col).v[i] == Byte(1)]:
                                                        if not board_completeSearch and self.__status == 2: 
                                                            broken = True
                                                            break
                                                        val += 1
                                                        t = _board.clone()
                                                        t.setval(row, col, val)
                                                        self.__solve(t, depth + 1)
                                                #if we broke out of the previous loop, we also want to break out of
                                                # this one. unfortunately, "break 2" seems to be invalid syntax.
                                                if broken: break
                                                #else: didntBreak = True
                                                #if not didntBreak: break

        def clone(self): # const
                ret = board()
                for row in range(9):
                        for col in range(9):
                                if self.final[row][col]:
                                        ret.setval(row, col, self.final[row][col])
                return ret

        def mergemask(self, row, col): # const
                return self.rows[row] & self.cols[col] & self.cels[self.cell(row)][self.cell(col)]

        def findmincounts(self): # const
                #compute the list of lenghths of merged masks
                masks : List[Tuple[int, int, int]] = []
                for row in range(9):
                        for col in range(9):
                                if self.final[row][col] == 0:
                                        numallowed = self.mergemask(row, col).cnt()
                                        masks.append((numallowed, row, col))
                #return the minimum number of allowed moves, and a list of cells which are
                # not currently occupied and which have that number of allowed moves
                return (min(masks)[0], [(i[1],i[2]) for i in masks if i[0] == min(masks)[0]])

        def stats(self):
                t = time.time() - self.__starttime if self.__status == 1 else self.__endtime - self.__starttime
                return {'max depth' : self.__maxdepth, 'turns' : self.__turns, 'backtracks' : self.__backtracks, 'elapsed time' : int(t), 'boards examined': len(self.examined), 'number of solutions' : len(self.solutions)}

        def onexit(self):
                self.__endtime = time.time()
                self.__status = 2

                if board_notifyOnCompletion: print(self.stats()['turns'])

if __name__ == '__main__':
    puzzle = board()
    puzzle.fread('testdata/b6.pz')
    print(puzzle.to_str())
    puzzle.solve()
