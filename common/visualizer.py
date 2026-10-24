import csv
from pathlib import Path

import matplotlib
import matplotlib.pyplot as plt

matplotlib.use("Agg")

BASE_DIR = Path(__file__).resolve().parent
CSV_FILE = BASE_DIR / "TSPA.csv"
RESULTS_DIR = BASE_DIR.parent / "3_local" / "results"


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


def visualize(nodes, solution, title, output_file):
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

    ax.set_title(title)
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_aspect("equal", adjustable="datalim")
    ax.grid(True, alpha=0.2)
    ax.legend()
    plt.tight_layout()
    fig.savefig(output_file, dpi=150)
    plt.close(fig)


if __name__ == "__main__":
    for solution_file in sorted(RESULTS_DIR.glob("*.txt")):
        csv_file = (
            BASE_DIR / "TSPB.csv"
            if solution_file.stem.endswith("B")
            else CSV_FILE
        )
        nodes = load_nodes(csv_file)
        solution = load_solution(solution_file)
        output_file = solution_file.with_suffix(".png")
        visualize(nodes, solution, solution_file.name, output_file)
        print(f"Saved {output_file}")