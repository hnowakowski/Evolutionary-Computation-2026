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

std::vector<int32_t> localSteepestEdges(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode, subalgo subalgo) {
	std::vector<int32_t> sol = subalgo(matrix, nodes, startingNode);
	std::array<int32_t, 200> solLookup;
	solLookup.fill(-1);
	for (size_t i = 0; i < sol.size(); i++) {
		solLookup[sol[i]] = i;
	}

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

int main() {
	std::vector<Node> nodes = getNodes(1);
	std::vector<std::vector<int32_t>> distMatrix = getDistMatrix(nodes);
	runTestCases(localSteepestEdges, distMatrix, nodes, "reuseA", "reuse", 1, randomSolution);
	nodes = getNodes(2);
	distMatrix = getDistMatrix(nodes);
	runTestCases(localSteepestEdges, distMatrix, nodes, "reuseB", "reuseB", 1, randomSolution);
	std::cin.get();
	return 0;
}
