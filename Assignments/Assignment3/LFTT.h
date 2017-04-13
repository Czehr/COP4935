#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <assert.h>
#include <set>
using namespace std;

const int OPERATIONS_PER_TRANSACTION = 100;

enum Status { // Statuses a function can be in
	success,
	fail,
	retry
};

enum TxStatus { // Transaction Status
	Active,
	Comitted,
	Aborted
};

enum OpType { // Operation Type
	Insert,
	Delete,
	Find
};

struct Operation {
	OpType type;
	int key;
};

struct Desc { // Descriptor
	int size;
	TxStatus status;
	Operation ops[OPERATIONS_PER_TRANSACTION];
};

struct NodeInfo { // Node Information
	Desc* desc;
	int opid;
};

/*struct Node {
	NodeInfo* info;
	int key;
};*/

struct HelpStack
{
	int index;
	Desc* descriptors[256];

	void init(){
		index = 0;
	}

	void push(Desc* desc){
		assert(index < 255);
		descriptors[index++] = desc;
	}

	void pop(){
		assert(index > 0);
		index--;
	}

	bool contains(Desc* desc){
		for(int i = 0; i < index; i++){
			if(descriptors[i] == desc)
				return true;
		}
		return false;
	}
};

