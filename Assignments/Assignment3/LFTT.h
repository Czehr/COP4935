#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <assert.h>
#include <set>
using namespace std;

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
	static size_t sizeOf(uint8_t size) {
		return sizeof(int) + sizeof(TxStatus) + sizeof(Operation) * size;
	}
	int size;
	TxStatus status;
	Operation* ops; // This array will be dynamically allocated when a transaction is made
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

