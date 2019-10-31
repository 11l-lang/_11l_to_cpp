# dijkstra shortest-distance algorithm
#
# (C) 2006 Gustavo J.A.M. Carneiro, licensed under the GPL version 2

from typing import List

UInt32 = int
UInt64 = int
random_seed : UInt32 = 0
def rnd():
    global random_seed
    random_seed = (1664525 * random_seed + 1013904223) & 0xFFFF_FFFF
    return random_seed

def randint(max) -> UInt32:
    return (UInt64(rnd()) * max) >> 32
def randfloat(max):
    return rnd()*(max/float(0x1_0000_0000))
def random_uniform(min, max):
    return randfloat(max - min) + min

class Vertex:
    name : str
    def __init__(self, name = ''):
        self.name = name

    def __lt__(self, v):
        return self.name < v.name

    def __eq__(self, v):
        return self.name == v.name

    def __hash__(self):
        return hash(self.name)

    def __str__(self):
        return self.name

    def __repr__(self):
        return self.name


class Edge:
    u : Vertex
    v : Vertex
    d : float
    def __init__(self, u, v, d):
        self.u = u
        self.v = v
        self.d = d
    def __str__(self):
        return "[%s --%3.2f--> %s]" % (self.u.name, self.d, self.v.name)
    # def __repr__(self):
    #     return str(self)

class Graph:
    v : List[Vertex]
    e : List[Edge]
    def __init__(self):
        v : List[Vertex] = []

        for n in range(100):
            v.append(Vertex(str(n + 1)))

        e : List[Edge] = []
        for n in range(10*len(v)):
            ui = randint(len(v))
            u = v[ui]
            tv = Vertex('')
            while True:
                tvi = randint(len(v))
                tv = v[tvi]
                if tvi != ui:
                    break
            e.append(Edge(u, tv, random_uniform(10, 100)))

        self.v = v
        self.e = e

    def distance(self, s, ss):
        global G
        d : float
        for edge in [e for e in G.e if e.u == s and e.v == ss[0]]:
            d = edge.d
            break
        else:
            assert(False)

        for u, v in zip(ss[:-1], ss[1:]):
            for edge in [e for e in G.e if e.u == u and e.v == v]:
                d += edge.d
                break
            else:
                assert(False)
        return d

G = Graph()

def Extract_Min(q : List[Vertex], d):
    m_is_none = True
    m = Vertex('')
    md = 1e50
    for u in q:
        if m_is_none:
            m = u
            m_is_none = False
            md = d[u]
        else:
            if d[u] < md:
                md = d[u]
                m = u
    q.remove(m)
    return m

def dijkstra(g, t, s):
    d : Dict[Vertex, float] = {}
    previous : Dict[Vertex, Vertex] = {}
    for v in g.v:
        d[v] = 1e50 # infinity
        previous[v] = Vertex('')
    #del v
    d[s] = 0
    ss : List[Vertex] = []
    Q = g.v[:]


    while len(Q):
        u = Extract_Min(Q, d)
        if u == t:
            break
        ss.append(u)
        for edge in [e for e in g.e if e.u == u]:
            if d[u] + edge.d < d[edge.v]:
                d[edge.v] = d[u] + edge.d
                previous[edge.v] = u

    ss.clear()
    u = t
    while previous[u].name != '':
        ss.insert(0, u)
        u = previous[u]
    return ss

if __name__ == '__main__':
    for n in range(100):
        G = Graph()
        si = randint(len(G.v))
        s = G.v[si]
        t = Vertex('')
        while True:
            ti = randint(len(G.v))
            t = G.v[ti]
            if ti != si:
                break
        ss = dijkstra(G, t, s)
        if len(ss):
            print("dijkstra %s ---> %s: %s %.9f" % (str(s), str(t), str(ss), G.distance(s, ss)))
            for inter in ss[:-1]:
                S1 = dijkstra(G, t, inter)
                print("\t => dijkstra %s ---> %s: %s %.9f" % (str(inter), str(t), str(S1), G.distance(inter, S1)))
                if S1 != ss[ (len(ss) - len(S1)) : ]:
                    print("************ ALARM! **************")
