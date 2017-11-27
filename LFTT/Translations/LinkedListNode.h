#pragma once

//#include "LFTTTypeDef.h"
#include <atomic>

class Node {
public:
//	std::atomic<NodeInfo*> info;
	int key;
	void *val;
	
	std::atomic<uintptr_t> next;
};
