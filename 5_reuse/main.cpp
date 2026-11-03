#include "../common/utils.h"
#include "../1_greedy/main.h"

// refer to slide 114 of the presentation
int32_t edgeSwapDelta(std::vector<std::vector<int32_t>>& matrix, std::vector<int32_t>& sol, size_t i, size_t j, size_t k, size_t l) {
	int32_t delta = 0;
	// i -> j, k -> l turns into i -> k, j -> l
	delta -= matrix[i][j];
	delta -= matrix[k][l];
	delta += matrix[i][k];
	delta += matrix[j][l];
	return delta;
}

int32_t interNodeSwapDelta(std::vector<std::vector<int32_t>>& matrix, std::vector<int32_t>& sol, std::vector<Node>& nodes, std::array<int32_t, 200>& solLookup, int32_t _i, int32_t j) {
	int32_t delta = 0;
	delta -= nodes[_i].cost;
	delta += nodes[j].cost;

	// converting global matrix idx into sol's idx
	int32_t i = solLookup[_i];

	// too many weird edge cases so i'm calculating the indices this way instead of explicit formulas in a thousand elseif blocks
	int32_t beforeI = (i == 0) ? sol.size() - 1 : i - 1;
	int32_t afterI = (i == sol.size() - 1) ? 0 : i + 1;
	int32_t x = sol[i];
	delta -= matrix[sol[beforeI]][x];
	delta -= matrix[x][sol[afterI]];
	delta += matrix[sol[beforeI]][j];
	delta += matrix[j][sol[afterI]];
	return delta;
}

struct Move {
	bool isEdge;
	std::array<int32_t, 2> nodeSwap{};
	std::array<int32_t, 4> edgeSwap{};
	int32_t delta;

	bool operator<(const Move& m) {
		return this->delta < m.delta;
	}

	Move(std::vector<std::vector<int32_t>>& matrix, std::vector<int32_t>& sol, std::vector<Node>& nodes, std::array<int32_t, 200>& solLookup, int32_t i, int32_t j) {
		nodeSwap = {i, j};
		delta = interNodeSwapDelta(matrix, sol, nodes, solLookup, i, j);
		isEdge = false;
	}

	Move(std::vector<std::vector<int32_t>>& matrix, std::vector<int32_t>& sol, int32_t i, int32_t j, int32_t k, int32_t l) {
		edgeSwap = { i, j, k, l };
		delta = edgeSwapDelta(matrix, sol, i, j, k, l);
		isEdge = true;
	}
};

std::vector<int32_t> localSteepestEdges(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	std::vector<int32_t> sol = subalgo(matrix, nodes, startingNode);
	std::array<int32_t, 200> solLookup;
	solLookup.fill(-1);
	for (size_t i = 0; i < sol.size(); i++) {
		solLookup[sol[i]] = i;
	}

	std::vector<Move> movePool;
	// node swaps
	for (size_t i = 0; i < sol.size(); i++) {
		for (size_t j = 0; j < matrix.size(); j++) {
			if (solLookup[j] != -1) continue;
			movePool.emplace_back(std::array<size_t, 2>{ (size_t)sol[i], j });
		}
	}

	// edge swaps
	for (size_t i = 0; i < sol.size(); i++) {
		size_t j = (i == sol.size() - 1) ? 0 : i + 1;
		for (size_t k = i + 2; k < sol.size(); k++) {
			size_t l = (k == sol.size() - 1) ? 0 : k + 1;
			// edge case where the edges are next to each other because of the loop
			if (i == 0 && l == 0) continue;
			edgeSwapPool.emplace_back(std::array<size_t, 4>{(size_t)sol[i], (size_t)sol[j], (size_t)sol[k], (size_t)sol[l]});
		}
	}

	// actual search
	while (true) {
		size_t bestIdx = 0;
		bool isInterBest = false;
		int32_t bestDelta = 0; // we will only consider strictly better neighbours

		// inter
		for (size_t idx = 0; idx < nodeSwapPoolInter.size(); idx++){
			auto [i, j] = nodeSwapPoolInter[idx];
			int32_t delta = interNodeSwapDelta(matrix, sol, nodes, solLookup, i, j);
			if (delta < bestDelta) {
				bestIdx = idx;
				bestDelta = delta;
				isInterBest = true;
			}
		}
		// intra
		for (size_t idx = 0; idx < edgeSwapPool.size(); idx++) {
			auto [i, j, k, l] = edgeSwapPool[idx];
			int32_t delta = edgeSwapDelta(matrix, sol, i, j, k, l);
			if (delta < bestDelta) {
				bestIdx = idx;
				bestDelta = delta;
				isInterBest = false;
			}
		}
		// apply change and reset the neighbourhood
		if (bestDelta != 0) {
			if (isInterBest) {
				auto [i, j] = nodeSwapPoolInter[bestIdx];
				solLookup[j] = solLookup[i];
				solLookup[i] = -1;
				std::replace(sol.begin(), sol.end(), i, j);
			}
			else {
				auto [i, j, k, l] = edgeSwapPool[bestIdx];
				size_t jIdx = solLookup[j];
				size_t kIdx = solLookup[k];
				std::reverse(sol.begin() + jIdx, sol.begin() + kIdx + 1);
				solLookup.fill(-1);
				for (size_t idx = 0; idx < sol.size(); idx++) {
					solLookup[sol[idx]] = idx;
				}
			}

			nodeSwapPoolInter.clear();
			edgeSwapPool.clear();

			for (size_t i = 0; i < sol.size(); i++) {
				for (size_t j = 0; j < matrix.size(); j++) {
					if (solLookup[j] != -1) continue;
					nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{ (size_t)sol[i], j });
				}
			}
			for (size_t i = 0; i < sol.size(); i++) {
				size_t j = (i == sol.size() - 1) ? 0 : i + 1;
				for (size_t k = i + 2; k < sol.size(); k++) {
					size_t l = (k == sol.size() - 1) ? 0 : k + 1;
					// edge case where the edges are next to each other because of the loop
					if (i == 0 && l == 0) continue;
					edgeSwapPool.emplace_back(std::array<size_t, 4>{(size_t)sol[i], (size_t)sol[j], (size_t)sol[k], (size_t)sol[l]});
				}
			}
		}
		else break;
	}
	return sol;
}

int main() {
	std::vector<Node> nodes = getNodes(1);
	std::vector<std::vector<int32_t>> distMatrix = getDistMatrix(nodes);
	runTestCases(localSteepestEdges, distMatrix, nodes, "candidates20A", "candidates20A", 1, randomSolution);
	nodes = getNodes(2);
	distMatrix = getDistMatrix(nodes);
	runTestCases(localSteepestEdges, distMatrix, nodes, "candidates20B", "candidates20B", 1, randomSolution);
	std::cin.get();
	return 0;
}
