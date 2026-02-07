import random
import sys

outfile = sys.argv[1]
E = int(sys.argv[2])      # 边数
V = int(sys.argv[3])      # 顶点数
mode = sys.argv[4]        # uniform / skewed

edges = []

for i in range(E):
    if mode == "uniform":
        u = random.randint(0, V - 1)
        v = random.randint(0, V - 1)
    elif mode == "skewed":
        # Zipf-like：少数点高度集中
        if random.random() < 0.7:
            u = random.randint(0, V // 10)
            v = random.randint(0, V // 10)
        else:
            u = random.randint(0, V - 1)
            v = random.randint(0, V - 1)
    else:
        raise ValueError("输入不符合")

    w = 1
    edges.append((u, v, w))

with open(outfile, "w") as f:
    for u, v, w in edges:
        f.write(f"{u} {v} {w}\n")

print("Generated", E, "edges.")
