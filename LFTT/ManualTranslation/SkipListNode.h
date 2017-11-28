#pragma once

#include "LFTTTypeDef.h"
#include <atomic>

static const int MAX_LEVEL = 5;

class Node {
public:
	std::atomic<NodeInfo*> info;
	int key = INT_MIN;
	void *val = nullptr;
	// TODO: Other info is needed for specific implementations. Use ROSE to modify?
	std::atomic<uintptr_t> next[MAX_LEVEL + 1]; // TODO: Initialize this array to all null?
	int topLevel = MAX_LEVEL;
};