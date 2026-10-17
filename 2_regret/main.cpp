#include "../common/utils.h"

// slot with the absolute lowest value on any node
// find some node that has the second lowest value on that slot
// do comparisons and stuff on those

// idea: pre-allocate matrix x resSize vector, where indices correspond with nodes and slots respectively
// after the main loop we have the absolute best slot and node
// look through the costs 3d vector on the best slot to find second best value, do comparisons for both on the entire insertion slot rows

// O(n^3)
std::vector<int32_t> nearestNeighbour2(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res;
	res.push_back(startingNode);
	std::vector<bool> contains(matrix.size());
	contains[startingNode] = true;
	std::vector<std::vector<int32_t>> insertionCosts(matrix.size(), std::vector<int32_t>(resSize));
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1;
		int32_t currCandidateChoice = -1;
		for (std::vector<int32_t>& v : insertionCosts) v.clear();
		for (int32_t j = 0; j < res.size() + 1; j++) { // slot
			for (int32_t k = 0; k < matrix.size(); k++) { //choice on each slot
				if (contains[k]) continue;
				if (j == 0) {
					if (matrix[k][res[j]] + nodes[k].cost < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k][j] = matrix[k][res[j]] + nodes[k].cost;
				}
				else if (j == res.size()) {
					if (matrix[res[j-1]][k] + nodes[k].cost < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[res[j-1]][k] + nodes[k].cost;
					}
					insertionCosts[k][j] = matrix[res[j - 1]][k] + nodes[k].cost;
				}
				else {
					if ((-matrix[res[j-1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k][j] = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
				}
			}
		}
		if (res.size() == 1) { // only one slot available so there's no regret to be had
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
			continue;
		}
		// finding 2nd best value for the chosen node
		int32_t no1BestCurrCost = insertionCosts[currCandidateChoice][currCandidateSlot];
		int32_t no2BestCurrCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts[currCandidateChoice].size(); j++) {
			if (j == currCandidateSlot) continue;
			no2BestCurrCost = std::min(no2BestCurrCost, insertionCosts[currCandidateChoice][j]);
		}

		// finding the 2nd best candidate for the chosen slot
		int32_t runnerUpIdx = -1;
		int32_t no1BestRunnerUpCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts.size(); j++) {
			if (j == currCandidateChoice || contains[j]) continue;
			if (insertionCosts[j][currCandidateSlot] < no1BestRunnerUpCost) {
				no1BestRunnerUpCost = insertionCosts[j][currCandidateSlot];
				runnerUpIdx = j;
			}
		}

		// runner up's 2nd best value
		int32_t no2BestRunnerUpCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts[runnerUpIdx].size(); j++) {
			if (j == currCandidateSlot) continue;
			no2BestRunnerUpCost = std::min(no2BestRunnerUpCost, insertionCosts[runnerUpIdx][j]);
		}

		if ((no2BestCurrCost - no1BestCurrCost) < (no2BestRunnerUpCost - no1BestRunnerUpCost)) {
			res.insert(res.begin() + currCandidateSlot, runnerUpIdx);
			contains[runnerUpIdx] = true;
		}
		else {
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
		}
	}
	return res;
}

// O(n^3)
std::vector<int32_t> greedyCycle2(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res;
	res.push_back(startingNode);
	std::vector<bool> contains(matrix.size());
	contains[startingNode] = true;
	std::vector<std::vector<int32_t>> insertionCosts(matrix.size(), std::vector<int32_t>(resSize));
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1; //where to insert
		int32_t currCandidateChoice = -1; //who to put in the insert
		for (std::vector<int32_t>& v : insertionCosts) v.clear();
		for (int32_t j = 0; j < res.size() + 1; j++) {
			for (int32_t k = 0; k < matrix.size(); k++) {
				if (contains[k]) continue;
				if (j == 0) {
					if ((-matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
				else if (j == res.size()) {
					if ((-matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost);
				}
				else {
					if ((-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
			}
		}
		if (res.size() == 1) { // only one slot available so there's no regret to be had
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
			continue;
		}
		// finding 2nd best value for the chosen node
		int32_t no1BestCurrCost = insertionCosts[currCandidateChoice][currCandidateSlot];
		int32_t no2BestCurrCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts[currCandidateChoice].size(); j++) {
			if (j == currCandidateSlot) continue;
			no2BestCurrCost = std::min(no2BestCurrCost, insertionCosts[currCandidateChoice][j]);
		}

		// finding the 2nd best candidate for the chosen slot
		int32_t runnerUpIdx = -1;
		int32_t no1BestRunnerUpCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts.size(); j++) {
			if (j == currCandidateChoice || contains[j]) continue;
			if (insertionCosts[j][currCandidateSlot] < no1BestRunnerUpCost) {
				no1BestRunnerUpCost = insertionCosts[j][currCandidateSlot];
				runnerUpIdx = j;
			}
		}

		// runner up's 2nd best value
		int32_t no2BestRunnerUpCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts[runnerUpIdx].size(); j++) {
			if (j == currCandidateSlot) continue;
			no2BestRunnerUpCost = std::min(no2BestRunnerUpCost, insertionCosts[runnerUpIdx][j]);
		}

		if ((no2BestCurrCost - no1BestCurrCost) < (no2BestRunnerUpCost - no1BestRunnerUpCost)) {
			res.insert(res.begin() + currCandidateSlot, runnerUpIdx);
			contains[runnerUpIdx] = true;
		}
		else {
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
		}
	}
	return res;
}

// both are the same as above, except the final regret comparison is different
// O(n^3)
std::vector<int32_t> nearestNeighbour2weighted(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode) {
	float alpha = 0.5;
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res;
	res.push_back(startingNode);
	std::vector<bool> contains(matrix.size());
	contains[startingNode] = true;
	std::vector<std::vector<int32_t>> insertionCosts(matrix.size(), std::vector<int32_t>(resSize));
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1;
		int32_t currCandidateChoice = -1;
		for (std::vector<int32_t>& v : insertionCosts) v.clear();
		for (int32_t j = 0; j < res.size() + 1; j++) { // slot
			for (int32_t k = 0; k < matrix.size(); k++) { //choice on each slot
				if (contains[k]) continue;
				if (j == 0) {
					if (matrix[k][res[j]] + nodes[k].cost < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(matrix[k][res[j]] + nodes[k].cost);
				}
				else if (j == res.size()) {
					if (matrix[res[j - 1]][k] + nodes[k].cost < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[res[j - 1]][k] + nodes[k].cost;
					}
					insertionCosts[k].push_back(matrix[res[j - 1]][k] + nodes[k].cost);
				}
				else {
					if ((-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
			}
		}
		if (res.size() == 1) { // only one slot available so there's no regret to be had
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
			continue;
		}
		// finding 2nd best value for the chosen node
		int32_t no1BestCurrCost = insertionCosts[currCandidateChoice][currCandidateSlot];
		int32_t no2BestCurrCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts[currCandidateChoice].size(); j++) {
			if (j == currCandidateSlot) continue;
			no2BestCurrCost = std::min(no2BestCurrCost, insertionCosts[currCandidateChoice][j]);
		}

		// finding the 2nd best candidate for the chosen slot
		int32_t runnerUpIdx = -1;
		int32_t no1BestRunnerUpCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts.size(); j++) {
			if (j == currCandidateChoice || contains[j]) continue;
			if (insertionCosts[j][currCandidateSlot] < no1BestRunnerUpCost) {
				no1BestRunnerUpCost = insertionCosts[j][currCandidateSlot];
				runnerUpIdx = j;
			}
		}

		// runner up's 2nd best value
		int32_t no2BestRunnerUpCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts[runnerUpIdx].size(); j++) {
			if (j == currCandidateSlot) continue;
			no2BestRunnerUpCost = std::min(no2BestRunnerUpCost, insertionCosts[runnerUpIdx][j]);
		}
		if (((no2BestCurrCost - no1BestCurrCost) * (1.0 - alpha)) + -(no1BestCurrCost * alpha) < ((no2BestRunnerUpCost - no1BestRunnerUpCost) * (1.0 - alpha)) + -(no1BestRunnerUpCost * alpha)) {
			res.insert(res.begin() + currCandidateSlot, runnerUpIdx);
			contains[runnerUpIdx] = true;
		}
		else {
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
		}
	}
	return res;
}

// O(n^3)
std::vector<int32_t> greedyCycle2weighted(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode) {
	float alpha = 0.5;
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res;
	res.push_back(startingNode);
	std::vector<bool> contains(matrix.size());
	contains[startingNode] = true;
	std::vector<std::vector<int32_t>> insertionCosts(matrix.size(), std::vector<int32_t>(resSize));
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1; //where to insert
		int32_t currCandidateChoice = -1; //who to put in the insert
		for (std::vector<int32_t>& v : insertionCosts) v.clear();
		for (int32_t j = 0; j < res.size() + 1; j++) {
			for (int32_t k = 0; k < matrix.size(); k++) {
				if (contains[k]) continue;
				if (j == 0) {
					if ((-matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
				else if (j == res.size()) {
					if ((-matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost);
				}
				else {
					if ((-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
			}
		}
		if (res.size() == 1) { // only one slot available so there's no regret to be had
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
			continue;
		}
		// finding 2nd best value for the chosen node
		int32_t no1BestCurrCost = insertionCosts[currCandidateChoice][currCandidateSlot];
		int32_t no2BestCurrCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts[currCandidateChoice].size(); j++) {
			if (j == currCandidateSlot) continue;
			no2BestCurrCost = std::min(no2BestCurrCost, insertionCosts[currCandidateChoice][j]);
		}

		// finding the 2nd best candidate for the chosen slot
		int32_t runnerUpIdx = -1;
		int32_t no1BestRunnerUpCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts.size(); j++) {
			if (j == currCandidateChoice || contains[j]) continue;
			if (insertionCosts[j][currCandidateSlot] < no1BestRunnerUpCost) {
				no1BestRunnerUpCost = insertionCosts[j][currCandidateSlot];
				runnerUpIdx = j;
			}
		}

		// runner up's 2nd best value
		int32_t no2BestRunnerUpCost = INT32_MAX;
		for (int32_t j = 0; j < insertionCosts[runnerUpIdx].size(); j++) {
			if (j == currCandidateSlot) continue;
			no2BestRunnerUpCost = std::min(no2BestRunnerUpCost, insertionCosts[runnerUpIdx][j]);
		}
		if (((no2BestCurrCost - no1BestCurrCost) * (1.0 - alpha)) + -(no1BestCurrCost * alpha) < ((no2BestRunnerUpCost - no1BestRunnerUpCost) * (1.0 - alpha)) + -(no1BestRunnerUpCost * alpha)) {
			res.insert(res.begin() + currCandidateSlot, runnerUpIdx);
			contains[runnerUpIdx] = true;
		}
		else {
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
		}
	}
	return res;
}

int main()
{
	std::vector<Node> nodes = getNodes(1);
	std::vector<std::vector<int32_t>> distMatrix = getDistMatrix(nodes);
	runTestCases(nearestNeighbour2, distMatrix, nodes, "nearestNeighbour2A", "nearestNeighbour2A", 4);
	runTestCases(greedyCycle2, distMatrix, nodes, "greedyCycle2A", "greedyCycle2A", 4);
	runTestCases(nearestNeighbour2weighted, distMatrix, nodes, "nearestNeighbour2weightedA", "nearestNeighbour2weightedA", 4);
	runTestCases(greedyCycle2weighted, distMatrix, nodes, "greedyCycle2weightedA", "greedyCycle2weightedA", 4);

	nodes = getNodes(2);
	distMatrix = getDistMatrix(nodes);
	runTestCases(nearestNeighbour2, distMatrix, nodes, "nearestNeighbour2B", "nearestNeighbour2B", 4);
	runTestCases(greedyCycle2, distMatrix, nodes, "greedyCycle2B", "greedyCycle2B", 4);
	runTestCases(nearestNeighbour2weighted, distMatrix, nodes, "nearestNeighbour2weightedB", "nearestNeighbour2weightedB", 4);
	runTestCases(greedyCycle2weighted, distMatrix, nodes, "greedyCycle2weightedB", "greedyCycle2weightedB", 4);

	std::cin.get();
	return 0;
}
