#pragma once
#include "../common/utils.h"

std::vector<int32_t> nearestNeighbour2(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode);
std::vector<int32_t> greedyCycle2(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode);
std::vector<int32_t> nearestNeighbour2weighted(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode);
std::vector<int32_t> greedyCycle2weighted(std::vector<std::vector<int32_t>>& matrix, std::vector<Node>& nodes, int32_t startingNode);