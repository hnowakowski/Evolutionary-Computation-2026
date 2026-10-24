#include "../common/utils.h"
#include "../1_greedy/main.h"
#include "../2_regret/main.h"
/*
 random from lab 1
 O(n)
std::vector<int32_t> randomSolution(std::vector<std::vector<int32_t>> matrix, int32_t startingNode)

 weighted greedy cycle with 2-regret from lab 2 <-- for TSPA
 O(n^3)
std::vector<int32_t> greedyCycle2weighted(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode)

 weighted 2-regret nearest neighbour from lab2 <-- for TSPB
 O(n^3)
std::vector<int32_t> nearestNeighbour2weighted(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode)

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

std::vector<int32_t> localGreedyEdges(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	std::mt19937 rng(startingNode);
	std::uniform_int_distribution<size_t> binaryDist(0, 1);
	std::vector<int32_t> sol = subalgo(matrix, nodes, startingNode);

	// setting up pools for random moves
	std::array<bool, 200> contains = {false};
	for (size_t i = 0; i < sol.size(); i++) {
		contains[sol[i]] = true;
	}

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

std::vector<int32_t> localGreedyNodes(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	std::vector<int32_t> startingSol = subalgo(matrix, nodes, startingNode);
	std::mt19937 rng(startingNode);
	std::uniform_int_distribution<size_t> binaryDist(0, 1);
	std::vector<int32_t> sol = subalgo(matrix, nodes, startingNode);

	// setting up pools for random moves
	std::array<bool, 200> contains = { false };
	std::vector<std::array<size_t, 2>> nodeSwapPoolIntra;
	for (size_t i = 0; i < sol.size() - 1; i++) {
		contains[sol[i]] = true;
		for (size_t j = i + 1; j < sol.size(); j++) {
			nodeSwapPoolIntra.emplace_back(std::array<size_t, 2>{(size_t)sol[i], (size_t)sol[j]});
		}
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

	// actual search
	while (!nodeSwapPoolInter.empty() || !nodeSwapPoolIntra.empty()) {
		bool improved = false;
		// inter
		if (binaryDist(rng) && !nodeSwapPoolInter.empty()) {
			std::uniform_int_distribution<size_t> dist(0, nodeSwapPoolInter.size() - 1);
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
			if (!nodeSwapPoolIntra.empty()) {
				std::uniform_int_distribution<size_t> dist(0, nodeSwapPoolIntra.size() - 1);
				size_t idx = dist(rng);
				auto [i, j] = nodeSwapPoolIntra[idx];
				nodeSwapPoolIntra[idx] = nodeSwapPoolIntra[nodeSwapPoolIntra.size() - 1];
				nodeSwapPoolIntra.pop_back();
				int32_t delta = intraNodeSwapDelta(matrix, sol, i, j);
				if (delta < 0) {
					size_t iIdx = std::find(sol.begin(), sol.end(), i) - sol.begin();
					size_t jIdx = std::find(sol.begin(), sol.end(), j) - sol.begin();
					std::swap(sol[iIdx], sol[jIdx]);
					improved = true;
				}
			}
		}
		// if nodes changed reset the neighbourhood
		if (improved) {
			nodeSwapPoolInter.clear();
			nodeSwapPoolIntra.clear();

			for (size_t i = 0; i < matrix.size(); i++) {
				if (!contains[i]) continue;
				for (size_t j = 0; j < matrix.size(); j++) {
					if (contains[j]) continue;
					nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{ i, j });
				}
			}
			for (size_t i = 0; i < sol.size() - 1; i++) {
				for (size_t j = i + 1; j < sol.size(); j++) {
					nodeSwapPoolIntra.emplace_back(std::array<size_t, 2>{(size_t)sol[i], (size_t)sol[j]});
				}
			}
		}
	}
	return sol;
}


std::vector<int32_t> localSteepestEdges(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	std::vector<int32_t> sol = subalgo(matrix, nodes, startingNode);

	std::array<bool, 200> contains = { false };
	for (size_t i = 0; i < sol.size(); i++) {
		contains[sol[i]] = true;
	}

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
		else break;
	}
	return sol;
}

std::vector<int32_t> localSteepestNodes(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	std::vector<int32_t> startingSol = subalgo(matrix, nodes, startingNode);
	std::vector<int32_t> sol = subalgo(matrix, nodes, startingNode);

	std::array<bool, 200> contains = { false };
	std::vector<std::array<size_t, 2>> nodeSwapPoolIntra;
	for (size_t i = 0; i < sol.size() - 1; i++) {
		contains[sol[i]] = true;
		for (size_t j = i + 1; j < sol.size(); j++) {
			nodeSwapPoolIntra.emplace_back(std::array<size_t, 2>{(size_t)sol[i], (size_t)sol[j]});
		}
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

	// actual search
	while (true) {
		size_t bestIdx = 0;
		bool isInterBest = false;
		int32_t bestDelta = 0; // we will only consider strictly better neighbours

		// inter
		for (size_t idx = 0; idx < nodeSwapPoolInter.size(); idx++) {
			auto [i, j] = nodeSwapPoolInter[idx];
			int32_t delta = interNodeSwapDelta(matrix, sol, nodes, i, j);
			if (delta < bestDelta) {
				bestIdx = idx;
				bestDelta = delta;
				isInterBest = true;
			}
		}
		// intra
		for (size_t idx = 0; idx < nodeSwapPoolIntra.size(); idx++) {
			auto [i, j] = nodeSwapPoolIntra[idx];
			int32_t delta = intraNodeSwapDelta(matrix, sol, i, j);
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
				auto [i, j] = nodeSwapPoolIntra[bestIdx];
				size_t iIdx = std::find(sol.begin(), sol.end(), i) - sol.begin();
				size_t jIdx = std::find(sol.begin(), sol.end(), j) - sol.begin();
				std::swap(sol[iIdx], sol[jIdx]);
			}

			nodeSwapPoolInter.clear();
			nodeSwapPoolIntra.clear();

			for (size_t i = 0; i < matrix.size(); i++) {
				if (!contains[i]) continue;
				for (size_t j = 0; j < matrix.size(); j++) {
					if (contains[j]) continue;
					nodeSwapPoolInter.emplace_back(std::array<size_t, 2>{ i, j });
				}
			}
			for (size_t i = 0; i < sol.size() - 1; i++) {
				for (size_t j = i + 1; j < sol.size(); j++) {
					nodeSwapPoolIntra.emplace_back(std::array<size_t, 2>{(size_t)sol[i], (size_t)sol[j]});
				}
			}
		}
		else break;
	}
	return sol;
}


int main()
{
	std::vector<Node> nodes = getNodes(1);
	std::vector<std::vector<int32_t>> distMatrix = getDistMatrix(nodes);
	runTestCases(localGreedyEdges, distMatrix, nodes, "localGreedyEdgesRandomA", "localGreedyEdgesRandomA", 8, randomSolution);
	runTestCases(localGreedyNodes, distMatrix, nodes, "localGreedyNodesRandomA", "localGreedyNodesRandomA", 8, randomSolution);
	runTestCases(localSteepestEdges, distMatrix, nodes, "localSteepestEdgesRandomA", "localSteepestEdgesRandomA", 8, randomSolution);
	runTestCases(localSteepestNodes, distMatrix, nodes, "localSteepestNodesRandomA", "localSteepestNodesRandomA", 8, randomSolution);

	runTestCases(localGreedyEdges, distMatrix, nodes, "localGreedyEdgesHeuristicA", "localGreedyEdgesHeuristicA", 8, greedyCycle2weighted);
	runTestCases(localGreedyNodes, distMatrix, nodes, "localGreedyNodesHeuristicA", "localGreedyNodesHeuristicA", 8, greedyCycle2weighted);
	runTestCases(localSteepestEdges, distMatrix, nodes, "localSteepestEdgesHeuristicA", "localSteepestEdgesHeuristicA", 8, greedyCycle2weighted);
	runTestCases(localSteepestNodes, distMatrix, nodes, "localSteepestNodesHeuristicA", "localSteepestNodesHeuristicA", 8, greedyCycle2weighted);

	nodes = getNodes(2);
	distMatrix = getDistMatrix(nodes);
	runTestCases(localGreedyEdges, distMatrix, nodes, "localGreedyEdgesRandomB", "localGreedyEdgesRandomB", 8, randomSolution);
	runTestCases(localGreedyNodes, distMatrix, nodes, "localGreedyNodesRandomB", "localGreedyNodesRandomB", 8, randomSolution);
	runTestCases(localSteepestEdges, distMatrix, nodes, "localSteepestEdgesRandomB", "localSteepestEdgesRandomB", 8, randomSolution);
	runTestCases(localSteepestNodes, distMatrix, nodes, "localSteepestNodesRandomB", "localSteepestNodesRandomB", 8, randomSolution);

	runTestCases(localGreedyEdges, distMatrix, nodes, "localGreedyEdgesHeuristicB", "localGreedyEdgesHeuristicB", 8, nearestNeighbour2weighted);
	runTestCases(localGreedyNodes, distMatrix, nodes, "localGreedyNodesHeuristicB", "localGreedyNodesHeuristicB", 8, nearestNeighbour2weighted);
	runTestCases(localSteepestEdges, distMatrix, nodes, "localSteepestEdgesHeuristicB", "localSteepestEdgesHeuristicB", 8, nearestNeighbour2weighted);
	runTestCases(localSteepestNodes, distMatrix, nodes, "localSteepestNodesHeuristicB", "localSteepestNodesHeuristicB", 8, nearestNeighbour2weighted);

	std::cin.get();
	return 0;
}
