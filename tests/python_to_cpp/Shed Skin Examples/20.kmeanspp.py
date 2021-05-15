# http://rosettacode.org/wiki/K-means%2B%2B_clustering

import math
import random
from copy import copy
from typing import NamedTuple

FLOAT_MAX = 1e100


class Point:
    x = 0.0
    y = 0.0
    group = 0


def generate_points(npoints, radius):
    points = [Point() for _ in range(npoints)]

    # note: this is not a uniform 2-d distribution
    for p in points:
        r = random.random() * radius
        ang = random.random() * 2 * math.pi
        p.x = r * math.cos(ang)
        p.y = r * math.sin(ang)

    return points

def sqr_distance_2D(a, b):
    return (a.x - b.x) ** 2  +  (a.y - b.y) ** 2

def nearest_cluster_center(point, cluster_centers):
    """Distance and index of the closest cluster center"""

    min_index = point.group
    min_dist = FLOAT_MAX

    for i, cc in enumerate(cluster_centers):
        d = sqr_distance_2D(cc, point)
        if min_dist > d:
            min_dist = d
            min_index = i

    return (min_index, min_dist)


def kpp(points : list, cluster_centers : list):
    cluster_centers[0] = copy(random.choice(points))
    d = [0.0 for _ in range(len(points))]

    for i in range(1, len(cluster_centers)):
        sum = 0.0
        for j, p in enumerate(points):
            d[j] = nearest_cluster_center(p, cluster_centers[:i])[1]
            sum += d[j]

        sum *= random.random()

        for j, di in enumerate(d):
            sum -= di
            if sum > 0:
                continue
            cluster_centers[i] = copy(points[j])
            break

    for p in points:
        p.group = nearest_cluster_center(p, cluster_centers)[0]


def lloyd(points : list, nclusters):
    cluster_centers = [Point() for _ in range(nclusters)]

    # call k++ init
    kpp(points, cluster_centers)

    lenpts10 = len(points) >> 10

    changed = 0
    while True:
        # group element for centroids are used as counters
        for cc in cluster_centers:
            cc.x = 0
            cc.y = 0
            cc.group = 0

        for p in points:
            cluster_centers[p.group].group += 1
            cluster_centers[p.group].x += p.x
            cluster_centers[p.group].y += p.y

        for cc in cluster_centers:
            cc.x /= cc.group
            cc.y /= cc.group

        # find closest centroid of each PointPtr
        changed = 0
        for p in points:
            min_i = nearest_cluster_center(p, cluster_centers)[0]
            if min_i != p.group:
                changed += 1
                p.group = min_i

        # stop when 99.9% of points are good
        if changed <= lenpts10:
            break

    for i, cc in enumerate(cluster_centers):
        cc.group = i

    return cluster_centers

class Color(NamedTuple):
    r : float
    g : float
    b : float

def print_eps(points, cluster_centers, w=400, h=400):
    colors : List[Color] = []
    for i in range(len(cluster_centers)):
        colors.append(Color((3 * (i + 1) % 11) / 11.0,
                            (7 * i % 11) / 11.0,
                            (9 * i % 11) / 11.0))

    max_x = -FLOAT_MAX
    max_y = -FLOAT_MAX
    min_x = FLOAT_MAX
    min_y = FLOAT_MAX

    for p in points:
        if max_x < p.x: max_x = p.x
        if min_x > p.x: min_x = p.x
        if max_y < p.y: max_y = p.y
        if min_y > p.y: min_y = p.y

    scale = min(w / (max_x - min_x),
                h / (max_y - min_y))
    cx = (max_x + min_x) / 2
    cy = (max_y + min_y) / 2

    print("%%!PS-Adobe-3.0\n%%%%BoundingBox: -5 -5 %d %d" % (w + 10, h + 10))

    print(("/l {rlineto} def /m {rmoveto} def\n" +
           "/c { .25 sub exch .25 sub exch .5 0 360 arc fill } def\n" +
           "/s { moveto -2 0 m 2 2 l 2 -2 l -2 -2 l closepath " +
           "   gsave 1 setgray fill grestore gsave 3 setlinewidth" +
           " 1 setgray stroke grestore 0 setgray stroke }def"))

    for i, cc in enumerate(cluster_centers):
        print(("%g %g %g setrgbcolor" %
               (colors[i].r, colors[i].g, colors[i].b)))

        for p in points:
            if p.group != i:
                continue
            print(("%.3f %.3f c" % ((p.x - cx) * scale + w / 2,
                                    (p.y - cy) * scale + h / 2)))

        print(("\n0 setgray %g %g s" % ((cc.x - cx) * scale + w / 2,
                                        (cc.y - cy) * scale + h / 2)))

    print("\n%%%%EOF")


def main():
    npoints = 30000
    k = 7 # # clusters

    points = generate_points(npoints, 10)
    cluster_centers = lloyd(points, k)
    print_eps(points, cluster_centers)


main()
