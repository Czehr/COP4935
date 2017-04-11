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
	int size;
	TxStatus status;
	Operation ops[];
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

/*Placeholders for "do_" functions
void do_locatePred(Node*& pred, Node*& curr, uint32_t key){
	return;
}

bool do_insert(uint32_t key, Desc* desc, uint8_t opid, Node*& inserted, Node*& pred){
	return true;
}

void do_delete(uint32_t key, Desc* desc, uint8_t opid, Node*& deleted, Node*& pred){
	return;
}*/
