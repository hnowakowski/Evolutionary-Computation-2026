#include "../common/utils.h"
#include "../1_greedy/main.h"
#include "../2_regret/main.h"
/*
 random from lab 1
 O(n)
std::vector<int32_t> randomSolution(std::vector<std::vector<int32_t>> matrix, int32_t startingNode)

 2-regret nearest neighbour from lab2
 O(n^3)
std::vector<int32_t> nearestNeighbour2(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode)

 weighted greedy cycle with 2-regret from lab 2
 O(n^3)
std::vector<int32_t> greedyCycle2weighted(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode)
*/


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

	delta -= matrix[sol[beforeI]][x];
	delta -= matrix[x][sol[afterI]];
	delta -= matrix[sol[beforeJ]][y];
	delta -= matrix[y][sol[afterJ]];
	delta += matrix[sol[beforeI]][y];
	delta += matrix[y][sol[afterI]];
	delta += matrix[sol[beforeJ]][x];
	delta += matrix[x][sol[afterJ]];
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

std::vector<int32_t> localGreedyEdges(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	std::mt19937 rng(startingNode);
	std::uniform_int_distribution<size_t> binaryDist(0, 1);
	std::vector<int32_t> sol = subalgo(matrix, nodes, startingNode);

	// setting up pools for random moves
	std::array<bool, 200> contains = {false};
	//std::vector<std::array<size_t, 2>> nodeSwapPoolIntra;
	for (size_t i = 0; i < sol.size() - 1; i++) {
		contains[sol[i]] = true;
		//for (size_t j = i + 1; j < sol.size(); j++) {
		//	nodeSwapPoolIntra.emplace_back(sol[i], sol[j]);
		//}
	}
	contains[sol[sol.size() - 1]] = true;

	std::vector<std::array<size_t, 2>> nodeSwapPoolInter;
	for (size_t i = 0; i < matrix.size(); i++) {
		if (!contains[i]) continue;
		for (size_t j = 0; j < matrix.size(); j++) {
			if (contains[j]) continue;
			nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{ i, j });
		}
	}
	std::vector<std::array<size_t, 4>> edgeSwapPool;
	for (size_t i = 0; i < sol.size(); i++) {
		size_t j = (i == sol.size() - 1) ? 0 : i + 1;
		for (size_t k = i + 2; k < sol.size(); k++) {
			size_t l = (k == sol.size() - 1) ? 0 : k + 1;
			// edge case where the edges are next to each other because of the loop
			if (i == 0 && l == 0) continue;
			edgeSwapPool.emplace_back(std::array<size_t, 4>{(size_t)sol[i], (size_t)sol[j], (size_t)sol[k], (size_t)sol[l]}); //debt of not using size_t everywhere from the start :(
		}
	}

	// actual search
	while (!nodeSwapPoolInter.empty() || !edgeSwapPool.empty()) {
		bool improved = false;
		// inter
		if (binaryDist(rng) && !nodeSwapPoolInter.empty()) {
				std::uniform_int_distribution<size_t> dist(0, nodeSwapPoolInter.size()-1);
				size_t idx = dist(rng);
				auto [i, j] = nodeSwapPoolInter[idx];
				nodeSwapPoolInter[idx] = nodeSwapPoolInter[nodeSwapPoolInter.size() - 1];
				nodeSwapPoolInter.pop_back();
				int32_t delta = interNodeSwapDelta(matrix, sol, nodes, i, j);
				if (delta < 0) {
					std::replace(sol.begin(), sol.end(), i, j);
					contains[i] = false;
					contains[j] = true;
					improved = true;
				}
		}
		// intra
		else {
			if (!edgeSwapPool.empty()) {
				std::uniform_int_distribution<size_t> dist(0, edgeSwapPool.size()-1);
				size_t idx = dist(rng);
				auto [i, j, k, l] = edgeSwapPool[idx];
				edgeSwapPool[idx] = edgeSwapPool[edgeSwapPool.size() - 1];
				edgeSwapPool.pop_back();
				int32_t delta = edgeSwapDelta(matrix, sol, i, j, k, l);
				size_t jIdx = std::find(sol.begin(), sol.end(), j) - sol.begin();
				size_t kIdx = std::find(sol.begin(), sol.end(), k) - sol.begin();
				if (delta < 0) {
					std::reverse(sol.begin() + jIdx, sol.begin() + kIdx + 1);
					improved = true;
				}
			}
		}
		// if nodes changed reset the neighbourhood
		if (improved) {
			nodeSwapPoolInter.clear();
			edgeSwapPool.clear();

			for (size_t i = 0; i < matrix.size(); i++) {
				if (!contains[i]) continue;
				for (size_t j = 0; j < matrix.size(); j++) {
					if (contains[j]) continue;
					nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{ i, j });
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
	}
	return sol;
}

//std::vector<int32_t> localGreedyNodes(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
//	std::vector<int32_t> startingSol = subalgo(matrix, nodes, startingNode);
//
//}
//
//std::vector<int32_t> localSteepestEdges(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
//
//}
//
//std::vector<int32_t> localSteepestNodes(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
//
//}


int main()
{
	std::vector<Node> nodes = getNodes(1);
	std::vector<std::vector<int32_t>> distMatrix = getDistMatrix(nodes);
	runTestCases(localGreedyEdges, distMatrix, nodes, "localGreedyEdgesA", "localGreedyEdgesA", 6, randomSolution);
	/*runTestCases(greedyCycle2, distMatrix, nodes, "greedyCycle2A", "greedyCycle2A", 4);
	runTestCases(nearestNeighbour2weighted, distMatrix, nodes, "nearestNeighbour2weightedA", "nearestNeighbour2weightedA", 4);
	runTestCases(greedyCycle2weighted, distMatrix, nodes, "greedyCycle2weightedA", "greedyCycle2weightedA", 4);*/

	nodes = getNodes(2);
	distMatrix = getDistMatrix(nodes);
	/*runTestCases(nearestNeighbour2, distMatrix, nodes, "nearestNeighbour2B", "nearestNeighbour2B", 4);
	runTestCases(greedyCycle2, distMatrix, nodes, "greedyCycle2B", "greedyCycle2B", 4);
	runTestCases(nearestNeighbour2weighted, distMatrix, nodes, "nearestNeighbour2weightedB", "nearestNeighbour2weightedB", 4);
	runTestCases(greedyCycle2weighted, distMatrix, nodes, "greedyCycle2weightedB", "greedyCycle2weightedB", 4);*/

	std::cin.get();
	return 0;
}
