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

std::vector<int32_t> localSteepestEdges(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	const size_t NEAREST_CNT = 20;
	std::vector<int32_t> sol = subalgo(matrix, nodes, startingNode);
	std::array<int32_t, 200> solLookup;
	solLookup.fill(-1);
	for (size_t i = 0; i < sol.size(); i++) {
		solLookup[sol[i]] = i;
	}

	std::array<std::array<int32_t, 199>, 200> nearestTemp;
	std::array<std::array<int32_t, NEAREST_CNT>, 200> nearest;
	for (size_t i = 0; i < matrix.size(); i++) {
		for (size_t j = 0; j < matrix.size(); j++) {
			if (j == i) continue;
			if (j > i) nearestTemp[i][j-1] = j;
			else nearestTemp[i][j] = j;
		}
	}

	for (size_t i = 0; i < matrix.size(); i++) {
		std::sort(nearestTemp[i].begin(), nearestTemp[i].end(),
			[&](int32_t idx1, int32_t idx2) {
				return (matrix[i][idx1] + nodes[idx1].cost) < (matrix[i][idx2] + nodes[idx2].cost);
			});
		for (size_t j = 0; j < NEAREST_CNT; j++) {
			nearest[i][j] = nearestTemp[i][j];
		}
	}


	std::vector<std::array<size_t, 2>> nodeSwapPoolInter;
	for (size_t i = 0; i < sol.size(); i++) {
		size_t beforeI = (i == 0) ? sol.size() - 1 : i - 1;
		size_t afterI = (i == sol.size() - 1) ? 0 : i + 1;
		for (int32_t j : nearest[sol[beforeI]]) {
			if (solLookup[j] != -1) continue;
			nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{(size_t)sol[i], (size_t)j});
		}
		for (int32_t j : nearest[sol[afterI]]) {
			if (solLookup[j] != -1) continue;
			nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{(size_t)sol[i], (size_t)j});
		}
	}

	std::vector<std::array<size_t, 4>> edgeSwapPool;
	for (size_t i = 0; i < sol.size(); i++) {
		size_t j = (i == sol.size() - 1) ? 0 : i + 1;
		size_t beforeI = (i == 0) ? (sol.size() - 1) : i - 1;
		for (int32_t k : nearest[sol[i]]) {
			if (solLookup[k] == -1) continue;
			size_t kIdx = solLookup[k];
			size_t beforeK = (kIdx == 0) ? (sol.size() - 1) : kIdx - 1;
			size_t l = (kIdx == sol.size() - 1) ? 0 : kIdx + 1;
			if (l != beforeI && j != beforeK && kIdx != j) {
				if (i < kIdx) { //just to make sure std reverse gets args in proper ordinality
					edgeSwapPool.emplace_back(std::array<size_t, 4>{(size_t)sol[i], (size_t)sol[j], (size_t)sol[kIdx], (size_t)sol[l]});
				}
				else {
					edgeSwapPool.emplace_back(std::array<size_t, 4>{(size_t)sol[kIdx], (size_t)sol[l], (size_t)sol[i], (size_t)sol[j]});
				}
			}
		}
	}
	//std::cout << "Node swap pool size: " << nodeSwapPoolInter.size() << "\n"; // ~1000 for 10, 10000 for normal, 20000 for 199
	//std::cout << "Edge swap pool size: " << edgeSwapPool.size() << "\n"; // ~420 for 10, 4850 for normal, 9600 for 199

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
				//size_t iIdx = solLookup[i];
				size_t jIdx = solLookup[j];
				size_t kIdx = solLookup[k];
				//size_t lIdx = solLookup[l];
				std::reverse(sol.begin() + jIdx, sol.begin() + kIdx + 1);
				solLookup.fill(-1);
				for (size_t idx = 0; idx < sol.size(); idx++) {
					solLookup[sol[idx]] = idx;
				}
			}

			nodeSwapPoolInter.clear();
			edgeSwapPool.clear();

			for (size_t i = 0; i < sol.size(); i++) {
				size_t beforeI = (i == 0) ? sol.size() - 1 : i - 1;
				size_t afterI = (i == sol.size() - 1) ? 0 : i + 1;
				for (int32_t j : nearest[sol[beforeI]]) {
					if (solLookup[j] != -1) continue;
					nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{(size_t)sol[i], (size_t)j});
				}
				for (int32_t j : nearest[sol[afterI]]) {
					if (solLookup[j] != -1) continue;
					nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{(size_t)sol[i], (size_t)j});
				}
			}
			for (size_t i = 0; i < sol.size(); i++) {
				size_t j = (i == sol.size() - 1) ? 0 : i + 1;
				size_t beforeI = (i == 0) ? (sol.size() - 1) : i - 1;
				for (int32_t k : nearest[sol[i]]) {
					if (solLookup[k] == -1) continue;
					size_t kIdx = solLookup[k];
					size_t beforeK = (kIdx == 0) ? (sol.size() - 1) : kIdx - 1;
					size_t l = (kIdx == sol.size() - 1) ? 0 : kIdx + 1;
					if (l != beforeI && j != beforeK && kIdx != j) {
						if (i < kIdx) { //just to make sure std reverse gets args in proper ordinality
							edgeSwapPool.emplace_back(std::array<size_t, 4>{(size_t)sol[i], (size_t)sol[j], (size_t)sol[kIdx], (size_t)sol[l]});
						}
						else {
							edgeSwapPool.emplace_back(std::array<size_t, 4>{(size_t)sol[kIdx], (size_t)sol[l], (size_t)sol[i], (size_t)sol[j]});
						}
					}
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
