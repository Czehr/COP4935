#pragma once

#include <atomic>
#include "LFTTTypeDef.h"

class Node {
public:
	std::atomic<NodeInfo*> info;
	int key;
	void *val;
	// TODO: Other info is needed for specific implementations. Use ROSE to modify?
	std::atomic<uintptr_t> next;
};

void init();
Node* Do_LocatePred(int key);
bool Do_Insert(Node* n);
bool Do_Delete(Node* n);
bool Do_Find(int key);