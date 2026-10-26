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


int32_t intraNodeSwapDelta(std::vector<std::vector<int32_t>>& matrix, std::vector<int32_t>& sol, size_t _i, size_t _j) {
	// converting global matrix idx into sol's idx
	int32_t i = std::find(sol.begin(), sol.end(), _i) - sol.begin(); //horrible
	int32_t j = std::find(sol.begin(), sol.end(), _j) - sol.begin(); //atrocious

	// too many weird edge cases so i'm calculating the indices this way instead of explicit formulas in a thousand elseif blocks
	int32_t beforeI = (i == 0) ? sol.size() - 1 : i - 1;
	int32_t afterI = (i == sol.size() - 1) ? 0 : i + 1;
	int32_t beforeJ = (j == 0) ? sol.size() - 1 : j - 1;
	int32_t afterJ = (j == sol.size() - 1) ? 0 : j + 1;
	int32_t delta = 0;
	int32_t x = sol[i];
	int32_t y = sol[j];

	// j right after i
	if (afterI == j) {
		delta -= matrix[sol[beforeI]][x];
		delta -= matrix[y][sol[afterJ]];
		delta += matrix[sol[beforeI]][y];
		delta += matrix[x][sol[afterJ]];
	}
	// j effectively right before i
	else if (i == 0 && j == sol.size() - 1) {
		delta -= matrix[sol[beforeJ]][y];
		delta -= matrix[x][sol[afterI]];

		delta += matrix[sol[beforeJ]][x];
		delta += matrix[y][sol[afterI]];
	}
	else {
	delta -= matrix[sol[beforeI]][x];
	delta -= matrix[x][sol[afterI]];
	delta -= matrix[sol[beforeJ]][y];
	delta -= matrix[y][sol[afterJ]];
	delta += matrix[sol[beforeI]][y];
	delta += matrix[y][sol[afterI]];
	delta += matrix[sol[beforeJ]][x];
	delta += matrix[x][sol[afterJ]];
	}
	return delta;
}

int32_t interNodeSwapDelta(std::vector<std::vector<int32_t>>& matrix, std::vector<int32_t>& sol, std::vector<Node>& nodes, int32_t _i, int32_t j) {
	int32_t delta = 0;
	delta -= nodes[_i].cost;
	delta += nodes[j].cost;

	// converting global matrix idx into sol's idx
	int32_t i = std::find(sol.begin(), sol.end(), _i) - sol.begin(); //deplorable

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
	const size_t NEAREST_CNT = 10;
	std::vector<int32_t> sol = subalgo(matrix, nodes, startingNode);
	std::array<bool, 200> contains = { false };
	for (size_t i = 0; i < sol.size(); i++) {
		contains[sol[i]] = true;
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
	for (size_t i = 0; i < matrix.size(); i++) {
		if (!contains[i]) continue;
		size_t iSolIdx = std::find(sol.begin(), sol.end(), i) - sol.begin();
		size_t beforeI = (iSolIdx == 0) ? sol.size() - 1 : iSolIdx - 1;
		size_t afterI = (iSolIdx == sol.size() - 1) ? 0 : iSolIdx + 1;
		for (size_t j = 0; j < nearest[beforeI].size(); j++) {
			if (contains[j]) continue;
			nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{i, j});
		}
		for (size_t j = 0; j < nearest[afterI].size(); j++) {
			if (contains[j]) continue;
			nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{i, j});
		}
	}

	std::vector<std::array<size_t, 4>> edgeSwapPool;
	for (size_t i = 0; i < sol.size(); i++) {
		size_t j = (i == sol.size() - 1) ? 0 : i + 1;
		for (size_t k = i + 2; k < sol.size(); k++) {
			size_t l = (k == sol.size() - 1) ? 0 : k + 1;
			// edge case where the edges are next to each other because of the loop
			if (i == 0 && l == 0) continue;
			if (std::find(nearest[sol[i]].begin(), nearest[sol[i]].end(), sol[j]) != nearest[sol[i]].end()
				|| std::find(nearest[sol[l]].begin(), nearest[sol[l]].end(), sol[k]) != nearest[sol[l]].end()) {
				edgeSwapPool.emplace_back(std::array<size_t, 4>{(size_t)sol[i], (size_t)sol[j], (size_t)sol[k], (size_t)sol[l]}); //debt of not using size_t everywhere from the start :(
			}
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
			int32_t delta = interNodeSwapDelta(matrix, sol, nodes, i, j);
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
				std::replace(sol.begin(), sol.end(), i, j);
				contains[i] = false;
				contains[j] = true;
			}
			else {
				auto [i, j, k, l] = edgeSwapPool[bestIdx];
				size_t jIdx = std::find(sol.begin(), sol.end(), j) - sol.begin();
				size_t kIdx = std::find(sol.begin(), sol.end(), k) - sol.begin();
				std::reverse(sol.begin() + jIdx, sol.begin() + kIdx + 1);
			}

			nodeSwapPoolInter.clear();
			edgeSwapPool.clear();

			for (size_t i = 0; i < matrix.size(); i++) {
				if (!contains[i]) continue;
				for (size_t j = 0; j < matrix.size(); j++) {
					if (contains[j]) continue;
					size_t iSolIdx = std::find(sol.begin(), sol.end(), i) - sol.begin();
					size_t beforeI = (iSolIdx == 0) ? sol.size() - 1 : iSolIdx - 1;
					size_t afterI = (iSolIdx == sol.size() - 1) ? 0 : iSolIdx + 1;
					if (std::find(nearest[sol[beforeI]].begin(), nearest[sol[beforeI]].end(), j) != nearest[sol[beforeI]].end()
						|| std::find(nearest[sol[afterI]].begin(), nearest[sol[afterI]].end(), j) != nearest[sol[afterI]].end()) {
						nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{i, j});
					}
				}
			}

			for (size_t i = 0; i < sol.size(); i++) {
				size_t j = (i == sol.size() - 1) ? 0 : i + 1;
				for (size_t k = i + 2; k < sol.size(); k++) {
					size_t l = (k == sol.size() - 1) ? 0 : k + 1;
					// edge case where the edges are next to each other because of the loop
					if (i == 0 && l == 0) continue;
					if (std::find(nearest[sol[i]].begin(), nearest[sol[i]].end(), j) != nearest[sol[i]].end()
						|| std::find(nearest[sol[l]].begin(), nearest[sol[l]].end(), k) != nearest[sol[l]].end()) {
						edgeSwapPool.emplace_back(std::array<size_t, 4>{(size_t)sol[i], (size_t)sol[j], (size_t)sol[k], (size_t)sol[l]}); //debt of not using size_t everywhere from the start :(
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
	runTestCases(localSteepestEdges, distMatrix, nodes, "localSteepestEdgesRandomA", "localSteepestEdgesRandomA", 8, randomSolution);
	nodes = getNodes(2);
	distMatrix = getDistMatrix(nodes);
	runTestCases(localSteepestEdges, distMatrix, nodes, "localSteepestEdgesRandomB", "localSteepestEdgesRandomB", 8, randomSolution);
	std::cin.get();
	return 0;
}
