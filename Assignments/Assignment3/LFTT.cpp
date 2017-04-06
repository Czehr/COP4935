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
}

struct NodeInfo { // Node Information
	Desc* desc;
	int opid;
}

struct Node {
	NodeInfo* info;
	int key;
}

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

	void Pop(){
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
}

// MarkableReference functions
static const uintptr_t mask = 1; // This mask is used to properly store our mark

// Take a pointer and a mark and pack them into one pointer
uintptr_t MarkableReference(Node *ref = NULL, bool mark = false) {
	return ((uintptr_t)ref & ~mask) | (mark ? 1 : 0);
}

// Get the pointer from our MarkableReference
Node* getReference(uintptr_t val) {
	return (Node*)(val & ~mask);
}

// Return whether a MarkableReference is marked or not
bool isMarked(uintptr_t val) {
	return (val & mask);
}

// Get the pointer from our MarkableReference and get our mark
Node *get(uintptr_t val, bool *mark) {
	*mark = isMarked(val);
	return (Node*)(val & ~mask);
}

isNodePresent(Node* n, int key) {
	return n.key == key;
}

bool isKeyPresent(NodeInfo* info, Desc* desc) {
	OpType op = info.desc.ops[info.opid];
	TxStatus status = info.desc.status;
	switch(status) {
		case: Active
			if(info.desc==desc)
				return op == Find || op == Insert;
			else
				return op == Find || op == Delete;
		case: Comitted
			return op == Find || op = Insert;
		case: Aborted
			return op == Find || op == Delete;
	}
}

Status updateInfo(Node* n, NodeInfo* info, bool wantKey) {
	NodeInfo* oldinfo = n.info;
	if(isMarked(oldinfo)) {
		do_delete(n); // TODO: Find a way to call the normal delete here
		return retry;
	}
	if(oldinfo.desc != info.desc) {
		executeOps(oldinfo.desc, oldinfo.opid + 1);
	} else if(oldinfo.opid >= info.opid) {
		return success;
	}
	bool hasKey = isKeyPresent(oldinfo); // TODO: What descriptor do we pass in isKeyPresent? Should have second argument
	if((!hasKey && wantKey) || (hasKey && !wantKey) {
		return fail;
	}
	if(info.desc.status != Active) {
		return fail;
	}
	if(n.info.compare_exchange_strong(oldinfo, info)) {
		return success;
	} else {
		return retry;
	}
}

bool insert(int key, Desc* desc, int opid) {
	NodeInfo* info = new NodeInfo;
	info.desc = desc;
	info.opid = opid;
	Status ret;
	while(true) {
		Node* curr = do_locatePred(key);
		if(isNodePresent(curr, key)) {
			ret = updateInfo(curr, info, false);
		} else {
			Node* n = new Node;
			n.key = key;
			n.info = info;
			bool insRet = do_insert(n);
			if(intRet)
				ret = success;
			else
				ret = fail;
		}
		if(ret == success) {
			return true;
		if(ret == fail) {
			return false;
		}
	}
}

bool find(int key, Desc* desc, int opid){
	NodeInfo* info = new NodeInfo;
	info.desc = desc;
	info.opid = opid;
	Status ret;
	while(true){
		Node* curr = do_locatePred(key);
		if(isNodePresent(curr, key))
			ret = updateInfo(curr, info, true)
		else
			ret = fail;
		if (ret == success)
			return true;
		else if (ret == fail)
			return false;
	}
}

bool deleteNode(int key, Desc* desc, int opid, Node* del){
	NodeInfo* info = new NodeInfo;
	info.desc = desc;
	info.opid = opid;
	Status ret;
	while(true){
		Node* curr = do_locatePred(key);
		if (isNodePresent(curr, key))
			ret = updateInfo(curr, info, true);
		else
			ret = fail;
		if (ret == success){
			del = curr;
			return true;
		}
		else if(ret == fail){
			del == NULL;
			return false;
		}
	}
}

void markDelete(set<Node> delnodes, Desc* desc){
	for (set<Node>::iterator del = delnodes.begin(); del!=delnodes.end(); ++i){
		if (del == NULL)
			continue;
		NodeInfo* info = del.info;
		if (info.desc != desc)
			continue;

		uintptr_t old = MarkableReference(del, false);
        uintptr_t altered = MarkableReference(del, true);
		if(del.compare_exchange_strong(old, altered))
			do_delete(del);
	}
}



//Placeholders for "do_" functions

Node* do_locatePred(key){
	Node temp;
	return &temp;
}

bool do_insert(Node* n){
	return true;
}

void do_delete(Node del){

}
