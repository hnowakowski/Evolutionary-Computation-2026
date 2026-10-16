#pragma once
#include <cstdint>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <thread>
#include <atomic>

struct Node {
	int32_t x;
	int32_t y;
	int32_t cost;

	void printNode();

	Node(int32_t x_in, int32_t y_in, int32_t cost_in);
};

int32_t dist(Node& n1, Node& n2);

std::vector<Node> getNodes(int32_t tspChoice = 1);

std::vector<std::vector<int32_t>> getDistMatrix(std::vector<Node> nodes);

std::vector<std::vector<int32_t>> getObjMatrix(std::vector<Node> nodes);

int32_t totalDist(std::vector<int32_t> solution, std::vector<std::vector<int32_t>> matrix);

int32_t totalCost(std::vector<int32_t> solution, std::vector<Node> nodes);

void saveSolution(std::vector<int32_t> solution, std::string filename);

void runTestCases(std::vector<int32_t>(*algo)(std::vector<std::vector<int32_t>>&, std::vector<Node>&, int32_t), std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes,
					std::string algoName, std::string filename, size_t threadCount = 4);