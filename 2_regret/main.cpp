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
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1;
		int32_t currCandidateChoice = -1;
		std::vector<std::vector<int32_t>> insertionCosts(matrix.size(), std::vector<int32_t>(res.size()+1));
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

		if ((no2BestCurrCost - no1BestCurrCost) > (no2BestRunnerUpCost - no1BestRunnerUpCost)) {
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
std::vector<int32_t> greedyCycle2(std::vector<std::vector<int32_t>> matrix, std::vector<Node> nodes, int32_t startingNode) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res;
	res.push_back(startingNode);
	std::vector<bool> contains(matrix.size());
	contains[startingNode] = true;
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1; //where to insert
		int32_t currCandidateChoice = -1; //who to put in the insert
		int32_t currRunnerUpSlot = -1;
		int32_t currRunnerUpChoice = -1;
		std::vector<std::vector<int32_t>> insertionCosts(matrix.size());
		for (int32_t j = 0; j < res.size() + 1; j++) {
			for (int32_t k = 0; k < matrix.size(); k++) {
				if (contains[k]) continue;
				if (j == 0) {
					if ((-matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
				else if (j == res.size()) {
					if ((-matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost) < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost);
				}
				else {
					if ((-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
			}
		}
		// regret
		std::vector<int32_t> bestCosts = insertionCosts[currCandidateChoice];
		std::vector<int32_t> runnerUpCosts = insertionCosts[currRunnerUpChoice];
		std::sort(bestCosts.begin(), bestCosts.end());
		std::sort(runnerUpCosts.begin(), runnerUpCosts.end());
		if ((bestCosts[1] - bestCosts[0]) > runnerUpCosts[1] - runnerUpCosts[0]) {
			res.insert(res.begin() + currRunnerUpSlot, currRunnerUpChoice);
			contains[currRunnerUpChoice] = true;
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
std::vector<int32_t> nearestNeighbour2weighted(std::vector<std::vector<int32_t>> matrix, std::vector<Node> nodes, int32_t startingNode, float alpha = 0.5) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res;
	res.push_back(startingNode);
	std::vector<bool> contains(matrix.size());
	contains[startingNode] = true;
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1;
		int32_t currCandidateChoice = -1;
		int32_t currRunnerUpSlot = -1;
		int32_t currRunnerUpChoice = -1;
		std::vector<std::vector<int32_t>> insertionCosts(matrix.size());
		for (int32_t j = 0; j < res.size() + 1; j++) { // slot
			for (int32_t k = 0; k < matrix.size(); k++) { //choice on each slot
				if (contains[k]) continue;
				if (j == 0) {
					if (matrix[k][res[j]] + nodes[k].cost < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(matrix[k][res[j]] + nodes[k].cost);
				}
				else if (j == res.size()) {
					if (matrix[res[j - 1]][k] + nodes[k].cost < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[res[j - 1]][k] + nodes[k].cost;
					}
					insertionCosts[k].push_back(matrix[res[j - 1]][k] + nodes[k].cost);
				}
				else {
					if ((-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
			}
		}
		if (res.size() == 1) {
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
			continue;
		}
		// the regret part
		std::vector<int32_t> bestCosts = insertionCosts[currCandidateChoice];
		std::vector<int32_t> runnerUpCosts = insertionCosts[currRunnerUpChoice];
		std::sort(bestCosts.begin(), bestCosts.end());
		std::sort(runnerUpCosts.begin(), runnerUpCosts.end());
		if (((bestCosts[1] - bestCosts[0]) * (1.0 - alpha)) + ((bestCosts[0]) * alpha) > ((runnerUpCosts[1] - runnerUpCosts[0]) * (1.0 - alpha)) + ((runnerUpCosts[0]) * alpha)) {
			res.insert(res.begin() + currRunnerUpSlot, currRunnerUpChoice);
			contains[currRunnerUpChoice] = true;
		}
		else {
			res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
			contains[currCandidateChoice] = true;
		}
	}
	return res;
}

// O(n^3)
std::vector<int32_t> greedyCycle2weighted(std::vector<std::vector<int32_t>> matrix, std::vector<Node> nodes, int32_t startingNode, float alpha = 0.5) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res;
	res.push_back(startingNode);
	std::vector<bool> contains(matrix.size());
	contains[startingNode] = true;
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1; //where to insert
		int32_t currCandidateChoice = -1; //who to put in the insert
		int32_t currRunnerUpSlot = -1;
		int32_t currRunnerUpChoice = -1;
		std::vector<std::vector<int32_t>> insertionCosts(matrix.size());
		for (int32_t j = 0; j < res.size() + 1; j++) {
			for (int32_t k = 0; k < matrix.size(); k++) {
				if (contains[k]) continue;
				if (j == 0) {
					if ((-matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
				else if (j == res.size()) {
					if ((-matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost) < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[res.size() - 1]][res[0]] + matrix[res[res.size() - 1]][k] + matrix[k][res[0]] + nodes[k].cost);
				}
				else {
					if ((-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost) < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost;
					}
					insertionCosts[k].push_back(-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]] + nodes[k].cost);
				}
			}
		}
		// regret
		std::vector<int32_t> bestCosts = insertionCosts[currCandidateChoice];
		std::vector<int32_t> runnerUpCosts = insertionCosts[currRunnerUpChoice];
		std::sort(bestCosts.begin(), bestCosts.end());
		std::sort(runnerUpCosts.begin(), runnerUpCosts.end());
		if (((bestCosts[1] - bestCosts[0]) * (1.0 - alpha)) + ((bestCosts[0]) * alpha) > ((runnerUpCosts[1] - runnerUpCosts[0]) * (1.0 - alpha)) + ((runnerUpCosts[0]) * alpha)) {
			res.insert(res.begin() + currRunnerUpSlot, currRunnerUpChoice);
			contains[currRunnerUpChoice] = true;
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
	auto start = std::chrono::steady_clock::now();
	runTestCases(nearestNeighbour2, distMatrix, nodes, "nearestNeighbour2", "nearest2A", 2);
	auto end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed = end - start;
	std::cout << "Elapsed: " << elapsed.count() << " seconds\n";
	// 1 thread:  0.918902 seconds
	// 2 threads: 0.475986 seconds
	// 4 threads: 0.252528 seconds
	// 8 threads: 0.186494 second


	std::cin.get();
	return 0;
}
