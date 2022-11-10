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

    def init_colors(self):
        # Color each node with its id
        self.vertex_color = defaultdict(int)
        for end in self.graph:
            self.add_vertex_color(end)
            for start in self.graph[end]:
                self.add_vertex_color(start)

    def find_unique_colors(self):
        unique_colors = list()
        for v in self.vertex_color:
            color = self.vertex_color[v]
            if color not in unique_colors:
                unique_colors.append(color)
        return unique_colors

    def bfs(self, vc, c):
        g_bfs = graph_bfs.GraphBFS(len(vc))
        for end in self.graph:
            if end in vc:
                for start in self.graph[end]:
                    g_bfs.add_edge(end, start)

        return g_bfs.bfs(c)

    def delete_found_scc(self, scvc):
        for v in scvc:
            self.graph.pop(v)
            self.V = self.V - 1

        for end in self.graph:
            start_list = self.graph[end]
            for i in range(len(start_list)):
                if start_list[i] in scvc:
                    start_list.pop(i)
                    i = i - 1

    def color_scc(self):
        while self.V:
            # init vertex_color
            self.init_colors()

            # spread vertex color fw until there are no changes in vertex_color
            changed_color = True
            while changed_color:
                changed_color = False
                for v in self.vertex_color:
                    for u in self.graph[v]:
                        if self.vertex_color[u] > self.vertex_color[v]:
                            self.vertex_color[v] = self.vertex_color[u]
                            changed_color = True

            # find all unique colors left in vertex_color
            unique_colors = self.find_unique_colors()

            for c in unique_colors:
                # find all vertexes with color c and put them in vc
                vc = list()
                for v in self.vertex_color:
                    if self.vertex_color[v] == c:
                        vc.append(v)

                # do bfs and find a strongly connected component
                scvc = self.bfs(vc, c)
                print(scvc)
                self.scc.append(scvc)
                self.SCC_counter = self.SCC_counter + 1

                # delete all items found in scvc
                self.delete_found_scc(scvc)


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



