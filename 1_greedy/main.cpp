#include "main.h"

// O(n)
std::vector<int32_t> randomSolution(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res(resSize);
	res[0] = startingNode;
	std::vector<int32_t> remainingPool;
	for (int32_t i = 0; i < resSize; i++) {
		if (i == startingNode) continue;
		remainingPool.emplace_back(i);
	}
	std::mt19937 rng(startingNode);
	for (int32_t i = 1; i < resSize; i++) {
		std::uniform_int_distribution<int32_t> dist(0, remainingPool.size() - 1);
		int32_t randomIdx = dist(rng);
		res[i] = remainingPool[randomIdx];
		remainingPool[randomIdx] = remainingPool[remainingPool.size() - 1];
		remainingPool.pop_back();
	}
	return res;
}

// just picking the lowest objective out of all the end's neighbours
// O(n^2)
std::vector<int32_t> nearestNeighbourEnd(std::vector<std::vector<int32_t>> matrix, int32_t startingNode) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res(resSize);
	std::vector<bool> contains(matrix.size());
	res[0] = startingNode;
	contains[startingNode] = true;
	int32_t curr = startingNode;
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidate = -1;
		for (int32_t j = 0; j < matrix.size(); j++) {
			if (j == curr || contains[j]) continue;
			if (matrix[curr][j] < minDist) {
				currCandidate = j;
				minDist = matrix[curr][j];
			}
		}
		curr = currCandidate;
		res[i] = curr;
		contains[curr] = true;
	}
	return res;
}

// so the idea is to just treat every space between nodes in the current solution as a possible
// slot for adding a connection in, taking the smallest dist out of all the connection possibilities from all slots possible
// O(n^3)
std::vector<int32_t> nearestNeighbourAll(std::vector<std::vector<int32_t>> matrix, int32_t startingNode) {
	int32_t resSize = (int32_t)std::ceil(matrix.size() / 2.0);
	std::vector<int32_t> res; //not pre-allocating space because i work on the temporary vector state and length on each pass and reallocation in this case is negeligible overhead)
	res.push_back(startingNode);
	std::vector<bool> contains(matrix.size());
	contains[startingNode] = true;
	for (int32_t i = 1; i < resSize; i++) {
		int32_t minDist = INT32_MAX;
		int32_t currCandidateSlot = -1; //where to insert
		int32_t currCandidateChoice = -1; //who to put in the insert
		for (int32_t j = 0; j < res.size() + 1; j++) { // +1 because of the space at the end
			for (int32_t k = 0; k < matrix.size(); k++) {
				if (contains[k]) continue;
				// delta from adding node k in spot j
				// given [x, y, z] if we want to insert w as such -> [x, w, y, z] then the delta will be -objective(x -> y) + objective(x -> w) + objective (w -> y)
				// obviously edge case for adding a node at the beginning or end, in which case the delta will be just +objective(w -> first node) or +objective(last node -> w)
				if (j == 0) {
					if (matrix[k][res[j]] < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[k][res[j]];
					}
				}
				else if (j == res.size()) {
					if (matrix[res[j-1]][k] < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = matrix[res[j-1]][k];
					}
				}
				else {
					if ((-matrix[res[j-1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]]) < minDist) {
						currCandidateSlot = j;
						currCandidateChoice = k;
						minDist = -matrix[res[j - 1]][res[j]] + matrix[res[j - 1]][k] + matrix[k][res[j]];
					}
				}
			}
		}
		res.insert(res.begin() + currCandidateSlot, currCandidateChoice);
		contains[currCandidateChoice] = true;
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


//int main()
//{
//	std::vector<Node> nodes = getNodes(2);
//	std::vector<std::vector<int32_t>> distMatrix = getDistMatrix(nodes);
//	std::vector<std::vector<int32_t>> objMatrix = getObjMatrix(nodes);
//
//	std::cout << "RANDOM\n";
//	std::vector<int32_t> bestSol;
//	int32_t bestObj = INT32_MAX;
//	int32_t worstObj = -1;
//	std::vector<int32_t> scores;
//	for (int32_t i = 0; i < 200; i++) {
//		std::vector<int32_t> tempSol = randomSolution(objMatrix, i);
//		int32_t obj = totalDist(tempSol, distMatrix) + totalCost(tempSol, nodes);
//		if (obj < bestObj) {
//			bestObj = obj;
//			bestSol = tempSol;
//		}
//		else worstObj = std::max(obj, worstObj);
//		scores.push_back(obj);
//	}
//	for each(int32_t var in bestSol){
//		std::cout << var << " ";
//	}
//	std::cout << "\nMax objective: " << worstObj;
//	std::cout << "\nAverage objective: " << std::accumulate(scores.begin(), scores.end(), 0)/scores.size();
//	std::cout << "\nMin objective: " << totalDist(bestSol, distMatrix) + totalCost(bestSol, nodes);
//	saveSolution(bestSol, "random.txt");
//	std::cout << "\n===============================================================\n";
//
//
//	std::cout << "Nearest neighbour end only\n";
//	bestSol.clear();
//	bestObj = INT32_MAX;
//	worstObj = -1;
//	scores.clear();
//	for (int32_t i = 0; i < 200; i++) {
//		std::vector<int32_t> tempSol = nearestNeighbourEnd(objMatrix, i);
//		int32_t obj = totalDist(tempSol, distMatrix) + totalCost(tempSol, nodes);
//		if (obj < bestObj) {
//			bestObj = obj;
//			bestSol = tempSol;
//		}
//		else worstObj = std::max(obj, worstObj);
//		scores.push_back(obj);
//	}
//	for each(int32_t var in bestSol) {
//		std::cout << var << " ";
//	}
//	std::cout << "\nMax objective: " << worstObj;
//	std::cout << "\nAverage objective: " << std::accumulate(scores.begin(), scores.end(), 0) / scores.size();
//	std::cout << "\nMin objective: " << totalDist(bestSol, distMatrix) + totalCost(bestSol, nodes);
//	saveSolution(bestSol, "nearestend.txt");
//	std::cout << "\n===============================================================\n";
//
//
//	std::cout << "Nearest neighbour all possibilities\n";
//	bestSol.clear();
//	bestObj = INT32_MAX;
//	worstObj = -1;
//	scores.clear();
//	for (int32_t i = 0; i < 200; i++) {
//		if (i % 20 == 0) std::cout << i << "\n";
//		std::vector<int32_t> tempSol = nearestNeighbourAll(objMatrix, i);
//		int32_t obj = totalDist(tempSol, distMatrix) + totalCost(tempSol, nodes);
//		if (obj < bestObj) {
//			bestObj = obj;
//			bestSol = tempSol;
//		}
//		else worstObj = std::max(obj, worstObj);
//		scores.push_back(obj);
//	}
//	for each(int32_t var in bestSol) {
//		std::cout << var << " ";
//	}
//	std::cout << "\nMax objective: " << worstObj;
//	std::cout << "\nAverage objective: " << std::accumulate(scores.begin(), scores.end(), 0) / scores.size();
//	std::cout << "\nMin objective: " << totalDist(bestSol, distMatrix) + totalCost(bestSol, nodes);
//	saveSolution(bestSol, "nearestall.txt");
//	std::cout << "\n===============================================================\n";
//
//
//	std::cout << "Greedy cycle\n";
//	bestSol.clear();
//	bestObj = INT32_MAX;
//	worstObj = -1;
//	scores.clear();
//	for (int32_t i = 0; i < 200; i++) {
//		if (i % 20 == 0) std::cout << i << "\n";
//		std::vector<int32_t> tempSol = greedyCycle(objMatrix, i);
//		int32_t obj = totalDist(tempSol, distMatrix) + totalCost(tempSol, nodes);
//		if (obj < bestObj) {
//			bestObj = obj;
//			bestSol = tempSol;
//		}
//		else worstObj = std::max(obj, worstObj);
//		scores.push_back(obj);
//	}
//	for each(int32_t var in bestSol) {
//		std::cout << var << " ";
//	}
//	std::cout << "\nMax objective: " << worstObj;
//	std::cout << "\nAverage objective: " << std::accumulate(scores.begin(), scores.end(), 0) / scores.size();
//	std::cout << "\nMin objective: " << totalDist(bestSol, distMatrix) + totalCost(bestSol, nodes);
//	saveSolution(bestSol, "greedycycle.txt");
//	std::cout << "\n===============================================================\n";
//
//	char buff;
//	std::cin >> buff;
//	return 0;
//}
