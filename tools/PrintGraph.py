import json

import networkx as nx
import networkx.drawing.nx_agraph as nx_agraph
import matplotlib.pyplot as plt
from pylab import rcParams
import os
from os.path import join as pjoin

root_dir = pjoin(os.path.dirname(__file__), "../")
intermediate_dir = pjoin(root_dir, "intermediate/")

G = nx.Graph()

nodes = []
labels = {}
links = []

def traverse(node, parent_index = -1):
    index = len(nodes)
    nodes.append(index)
    labels[index] = node["type"]
    if node["type"] == "TERM":
        labels[index] = node["value"]

    if parent_index != -1:
        links.append((parent_index, index))

    for child in node["children"]:
        traverse(child, index)


graph_file = pjoin(intermediate_dir, "graph.json")
with open(graph_file, "r") as f:
    if not f:
        print("Error: Can't open file '%s'" % graph_file)
        exit(1)

    j = json.load(f)
    raw = j["raw"]
    traverse(j["tree"])

for i, tuple in enumerate(nodes):
    G.add_node(tuple)

for link in links:
    G.add_edge(link[0], link[1])

rcParams['figure.figsize'] = 14, 10
pos = nx_agraph.graphviz_layout(G, prog='dot')
nx.draw(G, pos=pos,
        with_labels=True,
        labels=labels,
        node_color='none',
        node_size=1500,
        arrows=True,
        arrowstyle="-|>",
        arrowsize=10,
        )
plt.show()