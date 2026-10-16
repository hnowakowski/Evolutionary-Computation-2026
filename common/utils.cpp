#include "utils.h"

void Node::printNode() {
	std::cout << "x:" << this->x << " y:" << this->y << " cost:" << this->cost << "\n";
}

Node::Node(int32_t x_in, int32_t y_in, int32_t cost_in) :
	x(x_in), y(y_in), cost(cost_in)
{}

int32_t dist(Node& n1, Node& n2) {
	float x = std::powf((float)(n1.x - n2.x), 2.0);
	float y = std::powf((float)(n1.y - n2.y), 2.0);
	return (int32_t)(std::round(std::sqrt(x + y)));
}

std::vector<Node> getNodes(int32_t tspChoice) {
	std::string file = (tspChoice == 1) ? "../../../../common/TSPA.csv" : "../../../../common/TSPB.csv";
	std::fstream fin(file);
	std::vector<Node> res;
	std::string line;
	while (std::getline(fin, line)) {
		std::stringstream ss(line);
		std::string temp = "";
		Node newNode(-1, -1, -1);
		std::getline(ss, temp, ';');
		newNode.x = std::stoi(temp);
		std::getline(ss, temp, ';');
		newNode.y = std::stoi(temp);
		std::getline(ss, temp, ';');
		newNode.cost = std::stoi(temp);
		res.push_back(newNode);
	}
	return res;
}

// DON'T USE
std::vector<std::vector<int32_t>> getObjMatrix(std::vector<Node> nodes) {
	std::vector<std::vector<int32_t>> res(nodes.size());
	for (int32_t i = 0; i < nodes.size(); i++) {
		std::vector<int32_t> temp(nodes.size());
		for (int32_t j = 0; j < nodes.size(); j++) {
			temp[j] = dist(nodes[i], nodes[j]) + nodes[j].cost;
		}
		res[i] = temp;
	}
	return res;
}

std::vector<std::vector<int32_t>> getDistMatrix(std::vector<Node> nodes) {
	std::vector<std::vector<int32_t>> res(nodes.size());
	for (int32_t i = 0; i < nodes.size(); i++) {
		std::vector<int32_t> temp(nodes.size());
		for (int32_t j = 0; j < nodes.size(); j++) {
			temp[j] = dist(nodes[i], nodes[j]);
		}
		res[i] = temp;
	}
	return res;
}

int32_t totalDist(std::vector<int32_t> solution, std::vector<std::vector<int32_t>> matrix) {
	int32_t res = 0;
	for (int32_t i = 1; i < solution.size(); i++) {
		res += matrix[solution[i-1]][solution[i]];
	}
	res += matrix[solution[0]][solution[solution.size() - 1]];
	return res;
}

int32_t totalCost(std::vector<int32_t> solution, std::vector<Node> nodes) {
	int32_t res = 0;
	for (int32_t i = 0; i < solution.size(); i++) {
		res += nodes[solution[i]].cost;
	}
	return res;
}

void saveSolution(std::vector<int32_t> solution, std::string filename) {
	std::fstream fout;
	fout.open("../../../" + filename, std::ios::out);
	for each(auto var in solution){
		fout << var <<" ";
	}
	fout.close();
	std::cout << "\n../../../" + filename + " saved\n";
}

void runTestCases(std::vector<int32_t>(*algo)(std::vector<std::vector<int32_t>>&, std::vector<Node>&, int32_t), std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes,
					std::string algoName, std::string filename, size_t threadCount) {

	std::vector<std::vector<int32_t>> solutions(200);
	std::cout << algoName <<"\n";
	std::atomic<int32_t> currIdx(0);
	std::vector<std::thread> threads;

	for (int32_t i = 0; i < threadCount; i++) {
		threads.emplace_back([&]() {
			while (true) {
				int32_t curr = currIdx.fetch_add(1);
				if (curr % 20 == 0) std::cout << curr << "\n";
				if (curr >= 200) break;
				solutions[curr] = algo(matrix, nodes, curr);
			}
			});
	}

	for (size_t i = 0; i < threadCount; i++) {
		threads[i].join();
	}
	size_t bestSolIdx = -1;
	int32_t bestObj = INT32_MAX;
	std::vector<int32_t> scores(200);

	for (size_t i = 0; i < 200; i++) {
		scores[i] = totalCost(solutions[i], nodes) + totalDist(solutions[i], matrix);
		if (bestObj > scores[i]) {
			bestSolIdx = i;
			bestObj = scores[i];
		}
	}


	for (int32_t var : solutions[bestSolIdx]) {
		std::cout << var << " ";
	}
	std::cout << "\nMax objective: " << *std::max_element(scores.begin(), scores.end());
	std::cout << "\nAverage objective: " << std::accumulate(scores.begin(), scores.end(), 0) / scores.size();
	std::cout << "\nMin objective: " << bestObj;
	saveSolution(solutions[bestSolIdx], filename);
	std::cout << "\n===============================================================\n";
}