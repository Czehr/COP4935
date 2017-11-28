#ifndef LINKEDLISTNODE_H
#define LINKEDLISTNODE_H

#include "LFTTTypeDef.h"
#include <atomic>

class Node {
public:
	std::atomic<NodeInfo*> info;
	int key;
	void *val;
	// TODO: Other info is needed for specific implementations. Use ROSE to modify?
	std::atomic<uintptr_t> next;
};

#endif LINKEDLISTNODE_H