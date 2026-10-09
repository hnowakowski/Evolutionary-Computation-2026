import csv
import matplotlib.pyplot as plt

CSV_FILE = "TSPA.csv"
SOLUTION_FILE = "../1_greedy/greedycycle.txt"


def load_nodes(filename):
    """Load nodes in the same order as the C++ program."""
    nodes = []

    with open(filename, newline="") as file:
        reader = csv.reader(file, delimiter=";")

        for row in reader:
            if not row:
                continue

            x, y, cost = map(int, row[:3])
            nodes.append((x, y, cost))

    return nodes


def load_solution(filename):
    """Read whitespace-separated, zero-based node indices."""
    with open(filename) as file:
        return list(map(int, file.read().split()))


def visualize(nodes, solution):
    fig, ax = plt.subplots(figsize=(10, 8))

    xs = [node[0] for node in nodes]
    ys = [node[1] for node in nodes]

    ax.scatter(
        xs, ys,
        color="lightgray",
        s=35,
        label="Unselected nodes",
        zorder=2,
    )

    selected_x = [nodes[i][0] for i in solution]
    selected_y = [nodes[i][1] for i in solution]
    selected_costs = [nodes[i][2] for i in solution]

    all_costs = [node[2] for node in nodes]
    vmin = min(all_costs)
    vmax = max(all_costs)

    scatter = ax.scatter(
        selected_x,
        selected_y,
        c=selected_costs,
        cmap="viridis",
        vmin=vmin,
        vmax=vmax,
        s=65,
        edgecolors="black",
        linewidths=0.5,
        label="Selected nodes (color = cost)",
        zorder=3,
    )
    cbar = fig.colorbar(scatter, ax=ax)
    cbar.set_label("Node cost")

    # adding final edge back to start
    cycle = solution + [solution[0]]

    for a, b in zip(cycle[:-1], cycle[1:]):
        ax.plot(
            [nodes[a][0], nodes[b][0]],
            [nodes[a][1], nodes[b][1]],
            color="tomato",
            linewidth=1.5,
            zorder=1,
        )

    # Label selected nodes with their original indices.
    for i in solution:
        x, y, cost = nodes[i]

        ax.annotate(
            str(i),
            (x, y),
            xytext=(5, 5),
            textcoords="offset points",
            fontsize=9,
        )

    ax.set_title(SOLUTION_FILE)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_aspect("equal", adjustable="datalim")
    ax.grid(True, alpha=0.2)
    ax.legend()
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    nodes = load_nodes(CSV_FILE)
    solution = load_solution(SOLUTION_FILE)
    visualize(nodes, solution)