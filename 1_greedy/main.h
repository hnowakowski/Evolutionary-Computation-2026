#pragma once
#include "../common/utils.h"

std::vector<int32_t> randomSolution(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode);
std::vector<int32_t> nearestNeighbourEnd(std::vector<std::vector<int32_t>> matrix, int32_t startingNode);
std::vector<int32_t> nearestNeighbourAll(std::vector<std::vector<int32_t>> matrix, int32_t startingNode);
std::vector<int32_t> greedyCycle(std::vector<std::vector<int32_t>> matrix, int32_t startingNode);