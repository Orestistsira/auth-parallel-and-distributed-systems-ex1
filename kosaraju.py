# Kosaraju's algorithm to find strongly connected components in Python


from collections import defaultdict
from scipy.io import mmread

class Graph:

    def __init__(self, vertex):
        self.V = vertex
        self.graph = defaultdict(list)
        self.counter = 0

    # Add edge into the graph
    def add_edge(self, s, d):
        self.graph[s].append(d)

    # dfs
    def dfs(self, d, visited_vertex):
        visited_vertex[d] = True
        print(d, end=' ')
        for i in self.graph[d]:
            if not visited_vertex[i]:
                self.dfs(i, visited_vertex)

    def fill_order(self, d, visited_vertex, stack):
        visited_vertex[d] = True
        for i in self.graph[d]:
            if not visited_vertex[i]:
                self.fill_order(i, visited_vertex, stack)
        stack = stack.append(d)

    # transpose the matrix
    def transpose(self):
        g = Graph(self.V)

        for i in self.graph:
            for j in self.graph[i]:
                g.add_edge(j, i)
        return g

    # Print stongly connected components
    def print_scc(self):
        stack = []
        visited_vertex = [False] * (self.V)

        for i in range(self.V):
            if not visited_vertex[i]:
                self.fill_order(i, visited_vertex, stack)

        gr = self.transpose()

        visited_vertex = [False] * (self.V)

        while stack:
            i = stack.pop()
            if not visited_vertex[i]:
                gr.dfs(i, visited_vertex)
                print("")
                self.counter = self.counter + 1

        print("Conter: " + str(self.counter))

a = mmread('celegansneural.mtx')
array = a.toarray()
size = len(array)
g = Graph(size)

for i in range(0, size):
    for j in range(0, size):
        if array[i][j] != 0:
            g.add_edge(i, j)


print("Strongly Connected Components:")
g.print_scc()
