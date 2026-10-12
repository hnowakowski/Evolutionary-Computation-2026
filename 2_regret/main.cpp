#include "../common/utils.h"

// O(n^3)
std::vector<int32_t> nearestNeighbourGreedy2(std::vector<std::vector<int32_t>> matrix, int32_t startingNode) {
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
					if (matrix[k][res[j]] < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[k][res[j]];
					}
					insertionCosts[k].push_back(matrix[k][res[j]]);
				}
				else if (j == res.size()) {
					if (matrix[res[j-1]][k] < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[res[j-1]][k];
					}
					insertionCosts[k].push_back(matrix[res[j - 1]][k]);
				}
				else {
					if ((-matrix[res[j-1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]]) < minDist) {
						currRunnerUpSlot = currCandidateSlot;
						currRunnerUpChoice = currCandidateChoice;
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]];
					}
					insertionCosts[k].push_back(-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]]);
				}
			}
		}
		// the regret part
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

// same idea as above, except for the calculations in evaluating selecting a node
// we treat the current solution as a continuous cycle, so the formulas will be more generalized
// one caveat is that adding a node at the beginning is functionally the exact same as adding it at the end, so we skip considering nodes at the end
// O(n^3)
std::vector<int32_t> greedyCycle(std::vector<std::vector<int32_t>> matrix, int32_t startingNode) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res; //not pre-allocating space because i work on the temporary vector state and length on each pass and reallocation in this case is negeligible overhead)
	res.push_back(startingNode);
	std::vector<bool> contains(matrix.size());
	contains[startingNode] = true;
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1; //where to insert
		int32_t currCandidateChoice = -1; //who to put in the insert
		for (int32_t j = 0; j < res.size() ; j++) { // no +1 because we dont consider the space at the end
			for (int32_t k = 0; k < matrix.size(); k++) {
				if (contains[k]) continue;
				// delta from adding node k in spot j [x, y, z] -> [x, w, y, z]
				// generic formula: -objective(previous -> next) + objective(previous -> new) + objective (new -> next)
				// for nodes at the beginning, we consider the "previous" node as the currently last one
				if (j == 0) {
					if ((-matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]]) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[res.size() - 1]][res[j]] + matrix[res[res.size() - 1]][k] + matrix[k][res[j]];
					}
				}
				else {
					if ((-matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]]) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]];
					}
				}
			}
		}
		//std::cout << currCandidateSlot << "\n";
		res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
		contains[currCandidateChoice] = true;
	}
	return res;
}


int main()
{
	std::vector<Node> nodes = getNodes(1);
	std::vector<std::vector<int32_t>> distMatrix = getDistMatrix(nodes);
	std::vector<std::vector<int32_t>> objMatrix = getObjMatrix(nodes);
	std::vector<int32_t> bestSol;
	int32_t bestObj = INT32_MAX;
	int32_t worstObj = -1;
	std::vector<int32_t> scores;

	bestSol = nearestNeighbourGreedy2(objMatrix, 0);
	for each(auto var in bestSol)
	{
		std::cout << var << " ";
	}
	std::cout << "\n" << totalCost(bestSol, nodes) + totalDist(bestSol, distMatrix);

	/*std::cout << "Nearest neighbour all possibilities\n";
	bestSol.clear();
	bestObj = INT32_MAX;
	worstObj = -1;
	scores.clear();
	for (int32_t i = 0; i < 200; i++) {
		if (i % 20 == 0) std::cout << i << "\n";
		std::vector<int32_t> tempSol = nearestNeighbourAll(objMatrix, i);
		int32_t obj = totalDist(tempSol, distMatrix) + totalCost(tempSol, nodes);
		if (obj < bestObj) {
			bestObj = obj;
			bestSol = tempSol;
		}
		else worstObj = std::max(obj, worstObj);
		scores.push_back(obj);
	}
	for each(int32_t var in bestSol) {
		std::cout << var << " ";
	}
	std::cout << "\nMax objective: " << worstObj;
	std::cout << "\nAverage objective: " << std::accumulate(scores.begin(), scores.end(), 0) / scores.size();
	std::cout << "\nMin objective: " << totalDist(bestSol, distMatrix) + totalCost(bestSol, nodes);
	saveSolution(bestSol, "nearestall.txt");
	std::cout << "\n===============================================================\n";


	std::cout << "Greedy cycle\n";
	bestSol.clear();
	bestObj = INT32_MAX;
	worstObj = -1;
	scores.clear();
	for (int32_t i = 0; i < 200; i++) {
		if (i % 20 == 0) std::cout << i << "\n";
		std::vector<int32_t> tempSol = greedyCycle(objMatrix, i);
		int32_t obj = totalDist(tempSol, distMatrix) + totalCost(tempSol, nodes);
		if (obj < bestObj) {
			bestObj = obj;
			bestSol = tempSol;
		}
		else worstObj = std::max(obj, worstObj);
		scores.push_back(obj);
	}
	for each(int32_t var in bestSol) {
		std::cout << var << " ";
	}
	std::cout << "\nMax objective: " << worstObj;
	std::cout << "\nAverage objective: " << std::accumulate(scores.begin(), scores.end(), 0) / scores.size();
	std::cout << "\nMin objective: " << totalDist(bestSol, distMatrix) + totalCost(bestSol, nodes);
	saveSolution(bestSol, "greedycycle.txt");
	std::cout << "\n===============================================================\n";*/

	char buff;
	std::cin >> buff;
	return 0;
}
