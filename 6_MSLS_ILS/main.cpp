#include "../common/utils.h"

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
	// if edge then all 4 indices are just i, j, k, l
	// if node then i = original node, j = new node, k = original predecessor, l = original successor (if neighbours change then delta can become outdated)
	std::array<int32_t, 4> swap{};
	int32_t delta;

	bool operator<(const Move& m) const {
		return delta > m.delta;
	}

	Move(std::vector<std::vector<int32_t>>& matrix, std::vector<int32_t>& sol, std::vector<Node>& nodes, std::array<int32_t, 200>& solLookup, int32_t i, int32_t j) {
		int32_t iIdx = solLookup[i];
		int32_t beforeI = (iIdx == 0) ? sol.size() - 1 : iIdx - 1;
		int32_t afterI = (iIdx == sol.size() - 1) ? 0 : iIdx + 1;
		swap = {i, j, sol[beforeI], sol[afterI]};
		delta = interNodeSwapDelta(matrix, sol, nodes, solLookup, i, j);
		isEdge = false;
	}

	Move(std::vector<std::vector<int32_t>>& matrix, std::vector<int32_t>& sol, int32_t i, int32_t j, int32_t k, int32_t l) {
		swap = { i, j, k, l };
		delta = edgeSwapDelta(matrix, sol, i, j, k, l);
		isEdge = true;
	}
};

// 1 if original, -1 if reversed, 0 if non-existent
int32_t edgeState(std::vector<int32_t>& sol, std::array<int32_t, 200>& solLookup, int32_t i, int32_t j) {
	int32_t idxI = solLookup[i];
	int32_t idxJ = solLookup[j];
	if (idxI == -1 || idxJ == -1) return 0;
	int32_t afterI = (idxI == sol.size() - 1) ? 0 : idxI + 1;
	int32_t afterJ = (idxJ == sol.size() - 1) ? 0 : idxJ + 1;
	if (idxJ == afterI) return 1;
	if (idxI == afterJ) return -1;
	return 0;
}

void reverseWithCycle(std::vector<int32_t>& sol, std::array<int32_t, 200>& solLookup, int32_t start, int32_t end) {
	int32_t len = (end - start + sol.size()) % sol.size() + 1;

	for (int32_t i = 0; i < len / 2; i++) {
		int32_t a = (start + i) % sol.size();
		int32_t b = (start + len - 1 - i) % sol.size();

		std::swap(sol[a], sol[b]);
		solLookup[sol[a]] = a;
		solLookup[sol[b]] = b;
	}
}

std::vector<int32_t> localSearch(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, std::vector<int32_t>& startingSol, std::array<int32_t, 200>& solLookup) {
	std::vector<int32_t> sol = startingSol;

	std::priority_queue<Move> movePool;
	// node swaps
	for (size_t i = 0; i < sol.size(); i++) {
		for (size_t j = 0; j < matrix.size(); j++) {
			if (solLookup[j] != -1) continue;
			Move m(matrix, sol, nodes, solLookup, sol[i], j);
			if (m.delta < 0) movePool.emplace(m);
		}
	}

	// edge swaps
	for (size_t i = 0; i < sol.size(); i++) {
		size_t j = (i == sol.size() - 1) ? 0 : i + 1;
		for (size_t k = i + 2; k < sol.size(); k++) {
			size_t l = (k == sol.size() - 1) ? 0 : k + 1;
			if (i == 0 && l == 0) continue;
			Move m(matrix, sol, sol[i], sol[j], sol[k], sol[l]);
			if (m.delta < 0) movePool.emplace(m);
			m = Move(matrix, sol, sol[j], sol[i], sol[k], sol[l]);
			if (m.delta < 0) movePool.emplace(m);
		}
	}
	
	// actual search
	std::vector<Move> reservePool;
	int32_t iterCnt = 0;
	while (true) {
		iterCnt += 1;
		bool foundUsable = false;
		while (!movePool.empty()) {
			Move m = movePool.top();
			auto [i, j, k, l] = m.swap;
			if (!m.isEdge) {
				// nodes missing in sol or j present
				if (solLookup[i] == -1 || solLookup[j] != -1 || solLookup[k] == -1 || solLookup[l] == -1) {
					movePool.pop();
					continue;
				}
				// adjacent edges missing
				int32_t beforeI = (solLookup[i] == 0) ? sol.size() - 1 : solLookup[i] - 1;
				int32_t afterI = (solLookup[i] == sol.size() - 1) ? 0 : solLookup[i] + 1;
				if (sol[beforeI] != k || sol[afterI] != l) {
					movePool.pop();
					continue;
				}
				// swag
				sol[solLookup[i]] = j;
				solLookup[j] = solLookup[i];
				solLookup[i] = -1;
				movePool.pop();
				foundUsable = true;
				break;
			}
			else {
				int32_t state1 = edgeState(sol, solLookup, i, j);
				int32_t state2 = edgeState(sol, solLookup, k, l);
				// edges missing in sol
				if (state1 == 0 || state2 == 0) { 
					movePool.pop();
					continue;
				}
				// one edge reversed one not
				if (state1 != state2) { 
					movePool.pop();
					reservePool.emplace_back(m);
					continue;
				}
				// swag
				if (state1 == 1) {
					reverseWithCycle(sol, solLookup, solLookup[j], solLookup[k]);
				}
				else {
					reverseWithCycle(sol, solLookup, solLookup[k], solLookup[j]);
				}
				movePool.pop();
				foundUsable = true;
				break;
			}
		}
		// if applied a move, re-insert previously invalid moves and start evaluating moves from the beginning of the pool again
		if (foundUsable) {
			for (Move& m : reservePool) {
				movePool.emplace(m);
			}
			reservePool.clear();
		}

		// FULL RESET only if no usable moves found
		int32_t invalidCnt = 0;
		if (!foundUsable) {
			// unsure about this, could be an inefficiency but clearing this means i can use the pool being empty as a final condition for finishing if everything fails
			// idk how else to check finality with this approach where priority queue accepts duplicates by default
			reservePool.clear();
			// node swaps
			for (size_t i = 0; i < sol.size(); i++) {
				for (size_t j = 0; j < matrix.size(); j++) {
					if (solLookup[j] != -1) continue;
					Move m(matrix, sol, nodes, solLookup, sol[i], j);
					if (m.delta < 0) movePool.emplace(m);
				}
			}

			// edge swaps
			for (size_t i = 0; i < sol.size(); i++) {
				size_t j = (i == sol.size() - 1) ? 0 : i + 1;
				for (size_t k = i + 2; k < sol.size(); k++) {
					size_t l = (k == sol.size() - 1) ? 0 : k + 1;
					if (i == 0 && l == 0) continue;
					Move m(matrix, sol, sol[i], sol[j], sol[k], sol[l]);
					if (m.delta < 0) movePool.emplace(m);
					m = Move(matrix, sol, sol[j], sol[i], sol[k], sol[l]);
					if (m.delta < 0) {
						movePool.emplace(m);
						invalidCnt++;
					}
				}
			}
		}
		// no more improving moves can be found
		if (movePool.size() == invalidCnt && !foundUsable) break;
	}
	return sol;
}

std::vector<int32_t> randomSolutionSeed(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t seed) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res(resSize);
	std::vector<int32_t> remainingPool;
	for (int32_t i = 0; i < matrix.size(); i++) {
		remainingPool.emplace_back(i);
	}
	std::mt19937 rng(seed);
	for (int32_t i = 0; i < resSize; i++) {
		std::uniform_int_distribution<int32_t> dist(0, remainingPool.size() - 1);
		int32_t randomIdx = dist(rng);
		res[i] = remainingPool[randomIdx];
		remainingPool[randomIdx] = remainingPool[remainingPool.size() - 1];
		remainingPool.pop_back();
	}
	return res;
}


std::vector<int32_t> MSLS(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	const size_t ITER_CNT = 400;
	std::vector<int32_t> bestSol;
	int32_t bestScore = INT32_MAX;
	std::array<int32_t, 200> solLookup;
	for (size_t i = 0; i < ITER_CNT; i++) {
		std::vector<int32_t> sol = randomSolutionSeed(matrix, nodes, startingNode*400 + i);
		solLookup.fill(-1);
		for (size_t i = 0; i < sol.size(); i++) {
			solLookup[sol[i]] = i;
		}
		sol = localSearch(matrix, nodes, sol, solLookup);
		int32_t score = totalCost(sol, nodes) + totalDist(sol, matrix);
		//std::cout << score << "\n";
		if (score < bestScore) {
			bestScore = score;
			bestSol = sol;
		}
	}
	return bestSol;
}

// perturbation idea: 4 random moves
// DOES NOT take sol by reference
std::vector<int32_t> perturb(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, std::array<int32_t, 200>& solLookup, std::vector<int32_t> sol, int32_t seed) {
	const size_t MOVE_CNT = 4;
	std::mt19937 rng(seed);
	std::uniform_int_distribution<int32_t> typeDist(0, 1);
	std::uniform_int_distribution<int32_t> solDist(0, sol.size()-1);
	std::uniform_int_distribution<int32_t> matrixDist(0, matrix.size()-1);
	for (size_t i = 0; i < MOVE_CNT; i++) {
		int32_t type = typeDist(rng);
		// 0 = node
		if (type == 0) {
			int32_t i = solDist(rng); // sol idx
			int32_t j = matrixDist(rng); // matrix idx
			while (solLookup[j] != -1 || j == sol[i]) {
				j = matrixDist(rng);
			}
			solLookup[sol[i]] = -1;
			sol[i] = j;
			solLookup[j] = i; //warning? the expression param(1) is not true for this call
		}
		// 1 = edge
		else {
			std::uniform_int_distribution<int32_t> posDist(0, sol.size() - 6);
			int32_t i = posDist(rng);
			int32_t j = i + 1;
			std::uniform_int_distribution<int32_t> kDist(i + 2, sol.size() - 2);
			int32_t k = kDist(rng);
			reverseWithCycle(sol, solLookup, j, k);
		}
	}
	return sol;
}

std::vector<int32_t> ILS(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	const std::chrono::milliseconds TIME_LIMIT((long)((65.7921 + 66.4933)*1000./2./20.)); // should be a bit under 2s
	std::chrono::time_point start = std::chrono::steady_clock::now();
	std::vector<int32_t> sol = randomSolutionSeed(matrix, nodes, startingNode);
	int32_t bestScore = totalCost(sol, nodes) + totalDist(sol, matrix);
	std::array<int32_t, 200> solLookup;
	solLookup.fill(-1);
	for (size_t i = 0; i < sol.size(); i++) {
		solLookup[sol[i]] = i;
	}
	int32_t runCnt = 0;
	while ((std::chrono::steady_clock::now() - start) < TIME_LIMIT) {
		std::array<int32_t, 200> tempSolLookup = solLookup;
		std::vector<int32_t> tempSol = perturb(matrix, nodes, tempSolLookup, sol, runCnt);
		tempSol = localSearch(matrix, nodes, tempSol, tempSolLookup);
		int32_t score = totalCost(tempSol, nodes) + totalDist(tempSol, matrix);
		if (score < bestScore) {
			sol = tempSol;
			solLookup = tempSolLookup;
			bestScore = score;
		}
		runCnt++;
	}
	std::cout<< "Run count: " << runCnt << "\n";
	return sol;
}

// 67.3089 seconds
// 66.6899 seconds
int main() {
	std::vector<Node> nodes = getNodes(1);
	std::vector<std::vector<int32_t>> distMatrix = getDistMatrix(nodes);
	//runTestCases(MSLS, distMatrix, nodes, "MSLSA", "MSLSA", 1);
	//178 1 3 50 47 130 159 146 170 153 193 78 88 107 64 57 182 111 192 158 134 84 58 181 114 25 98 87 141 30 161 187 189 2 171 92 69 127 97 95 168 119 63 149 116 60 49 160 123 100 152 108 48 80 91 157 71 4 76 132 51 16 35 156 180 28 102 185 45 135 73 140 53 179 144 67 52 177 36 131 33 9 54 8 174 176 150 151 42 89 113 106 43 93 85 164 186 136 29 101
	//Max objective : 74050
	//Average objective : 73630
	//Min objective : 73285
	//Elapsed : 65.7921 seconds
	runTestCases(ILS, distMatrix, nodes, "ILSA", "ILSA", 1);
	nodes = getNodes(2);
	distMatrix = getDistMatrix(nodes);
	//runTestCases(MSLS, distMatrix, nodes, "MSLSB", "MSLSB", 1);
	//114 148 143 10 78 54 162 192 80 127 129 90 153 14 2 20 74 171 99 67 24 34 179 87 82 85 51 167 73 59 151 3 184 86 46 72 117 8 120 133 136 172 160 69 183 30 21 38 47 140 154 26 125 177 170 115 97 155 71 40 60 157 45 134 175 62 36 197 6 29 132 103 178 19 65 145 28 149 146 37 116 147 42 121 198 96 48 156 89 57 104 190 70 187 81 94 105 61 139 196
	//Max objective : 50427
	//Average objective : 50207
	//Min objective : 49655
	//Elapsed : 66.4933 seconds
	runTestCases(ILS, distMatrix, nodes, "ILSB", "ILSB", 1);
	std::cin.get();
	return 0;
}
