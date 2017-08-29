#pragma once

#include <atomic>
#include "LFTTTypeDef.h"

class Node {
public:
	std::atomic<NodeInfo*> info;
	int key;
	// TODO: Other info is needed for specific implementations. Use ROSE to modify?
	std::atomic<uintptr_t> next;
};

void init();
Node* Do_LocatePred(int key);
Status Do_Insert(Node* n);
Status Do_Delete(Node* n);