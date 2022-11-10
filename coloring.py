from collections import defaultdict
from scipy.io import mmread
import graph_bfs


class Graph:

    def __init__(self, vertex):
        self.V = vertex
        self.graph = defaultdict(list)
        self.vertex_color = defaultdict(int)
        self.SCC_counter = 0
        self.scc = list()

    # Add edge into the graph
    def add_edge(self, s, d):
        self.graph[s].append(d)

    def add_vertex_color(self, v):
        if v not in self.vertex_color:
            self.vertex_color[v] = v

    def color_scc(self):
        while self.V:
            # Color each node with its id
            self.vertex_color = defaultdict(int)
            for end in self.graph:
                self.add_vertex_color(end)
                for start in self.graph[end]:
                    self.add_vertex_color(start)

            changed_color = True
            while changed_color:
                changed_color = False
                for v in self.vertex_color:
                    for u in self.graph[v]:
                        if self.vertex_color[u] > self.vertex_color[v]:
                            self.vertex_color[v] = self.vertex_color[u]
                            changed_color = True

            unique_colors = list()
            for v in self.vertex_color:
                color = self.vertex_color[v]
                if color not in unique_colors:
                    unique_colors.append(color)

            for c in unique_colors:
                vc = list()
                for v in self.vertex_color:
                    if self.vertex_color[v] == c:
                        vc.append(v)

                g_bfs = graph_bfs.GraphBFS(len(vc))
                for end in self.graph:
                    if end in vc:
                        for start in self.graph[end]:
                            g_bfs.add_edge(end, start)

                scvs = g_bfs.bfs(c)
                print(scvs)
                self.scc.append(scvs)
                self.SCC_counter = self.SCC_counter + 1

                # delete all items in scvs
                for v in scvs:
                    self.graph.pop(v)
                    self.V = self.V - 1

                for end in self.graph:
                    l = self.graph[end]
                    for i in range(len(l)):
                        if l[i] in scvs:
                            l.pop(i)
                            i = i - 1


g = Graph(8)
g.add_edge(0, 1)
g.add_edge(1, 2)
g.add_edge(2, 3)
g.add_edge(2, 4)
g.add_edge(3, 0)
g.add_edge(4, 5)
g.add_edge(5, 6)
g.add_edge(6, 4)
g.add_edge(6, 7)

g.color_scc()

print(g.graph)
print(g.vertex_color)
print(g.V)
print(g.scc)
print(g.SCC_counter)



