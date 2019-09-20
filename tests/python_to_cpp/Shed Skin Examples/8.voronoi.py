# Textual Voronoi code modified from: <abhishek@ocf.berkeley.edu>
# http://www.ocf.berkeley.edu/~Eabhishek/

import random # for generateRandomPoints
import math

def generateRandomPoints(npoints=6):
    """Generate a few random points v1...vn"""
    print(str(npoints) + " points x,y:")
    points : List[Tuple[float, float]] = []
    for i in range(npoints):
        (xrand, yrand) = (random.random(), random.random())
        print(str(xrand) + ' ' + str(yrand))
        for xoff in range(-1, 2):
            for yoff in range(-1, 2):
                points.append( (xrand + xoff, yrand + yoff) )
    return points


def closest(x,y,points):
    """Function to find the closest of the vi."""
    (best, good) = (99.0*99.0, 99.0*99.0)
    for px, py in points:
        dist = (x-px)*(x-px) + (y-py)*(y-py)
        if dist < best:
            (best, good) = (dist, best)
        elif dist < good:
            good = dist
    return math.sqrt(best) / math.sqrt(good)


def generateScreen(points, rows=40, cols=80):
    yfact = 1.0 / cols
    xfact = 1.0 / rows
    screen : List[str] = []
    chars = " -.,+*$&#~~"
    for i in range(rows):
        x = i*xfact
        line = [ chars[int(10*closest(x, j*yfact, points))] for j in range(cols) ]
        screen.extend( line )
        screen.append("\n")
    return "".join(screen)


import time

if __name__ == '__main__':
    points = generateRandomPoints(10)
    print()
    t1 = time.perf_counter()
    print(generateScreen(points, 40, 80))
    t2 = time.perf_counter()
    print(round(t2-t1, 3))

