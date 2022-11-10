from collections import defaultdict


class GraphBFS:

    # Constructor
    def __init__(self, vertex):
        self.V = vertex
        # default dictionary to store graph
        self.graph = defaultdict(list)

    # function to add an edge to graph
    def add_edge(self, u, v):
        self.graph[u].append(v)

    # Function to print a BFS of graph
    def bfs(self, s):

        # Mark all the vertices as not visited
        visited = [False] * self.V

        # Create a queue for BFS
        queue = []

        # Mark the source node as
        # visited and enqueue it
        queue.append(s)
        visited[s] = True

        final_list = list()

        while queue:

            # Dequeue a vertex from
            # queue and print it
            s = queue.pop(0)
            #print(s, end=" ")
            final_list.append(s)

            # Get all adjacent vertices of the
            # dequeued vertex s. If a adjacent
            # has not been visited, then mark it
            # visited and enqueue it
            for i in self.graph[s]:
                if visited[i] == False:
                    queue.append(i)
                    visited[i] = True

        return final_list
