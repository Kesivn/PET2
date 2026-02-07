import random

V = 20
E = 50

edges = []
deg_out = [0] * V
deg_in = [0] * V
edge_map = {}

for _ in range(E):
    u = random.randint(0, V - 1)
    v = random.randint(0, V - 1)
    w = random.randint(1, 5)

    edges.append((u, v, w))
    deg_out[u] += w
    deg_in[v] += w
    edge_map[(u, v)] = edge_map.get((u, v), 0) + w

with open("truth_edges.txt", "w") as f:
    for u, v, w in edges:
        f.write(f"{u} {v} {w}\n")

with open("truth_answer.txt", "w") as f:
    f.write("Edge Weights:\n")
    for k, v in edge_map.items():
        f.write(f"{k[0]} {k[1]} = {v}\n")

    f.write("\nOut Degrees:\n")
    for i in range(V):
        f.write(f"{i} : {deg_out[i]}\n")

    f.write("\nIn Degrees:\n")
    for i in range(V):
        f.write(f"{i} : {deg_in[i]}\n")

print("Generated truth_edges.txt and truth_answer.txt")
