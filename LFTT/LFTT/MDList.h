#pragma once

#include <atomic>
#include "LFTTTypeDef.h"

// Note: Minimize the amount of data in this header to prevent filling up the namespace. 
// Only include what is needed to have a working node class. 
// Other constants, strucutres, etc. belong in MDList.cpp. 

class Node;

// The number of dimensions.
const int D = 10;
// The key range.
const int N = 1000;

struct AdoptDesc {
	Node *curr;
	int dp;
	int dc;
};

class Node {
public:
	std::atomic<NodeInfo*> info;
	int key;
	// TODO: Other info is needed for specific implementations. Use ROSE to modify?
	int k[D];
	void *val; // Value
	std::atomic<Node*> child[D];
	AdoptDesc * adesc;
};

void init();
Node* Do_LocatePred(int key);
bool Do_Insert(Node* node);
bool Do_Delete(Node* n);
bool Do_Find(int key);