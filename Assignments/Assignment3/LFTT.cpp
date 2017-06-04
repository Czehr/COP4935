#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <set>
#include "LFTT.h"
using namespace std;

// Following three are adjustable to run the program
// with different sets of functionality
const int THREAD_COUNT = 4;
const int TRANSACTION_SIZE = 1;
const int PERCENT_INSERT = 33;
const int PERCENT_DELETE = 33;
// Because of the way our random function works, PERCENT_FIND does nothing. 
// Set it by leaving a percentage left over from insert and delete. 
const int PERCENT_FIND = 33;
const int KEY_RANGE = 10000;
const int NUM_TRANSACTIONS = 320000;
const bool DEBUG = false;

class MarkableReference;
class Node;
class Pool;
class List;

static Node* getNode(int, int, int, Node*);
static Node* getNode2(int, int, int, int, Node*);

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

// Get the pointer from our MarkableReference and get our mark
Node *get(uintptr_t val, bool *mark) {
	*mark = (val & mask);
	return (Node*)(val & ~mask);
}

// Return whether a MarkableReference is marked or not
bool isMarked(NodeInfo* val) {
	return ((long)val & 1);
}

//Additional LFTT functionality

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


class Node {
public:
	int item;
	int key;
	atomic<NodeInfo*> info; //Points to NodeInfo struct
	atomic<uintptr_t> next;

	Node() {
		item = INT_MIN + 1;
		key = item;
	}
	Node(int num) {
		item = num;
		key = num;
	}
	Node(int num, Node *succ) {
		item = num;
		key = num;
		next.store(MarkableReference(succ));
	}
};

static void find(Node *head, int key, Node *&pred, Node *&curr) {
	Node *succ;
	bool marked = false;

	retry: while (true) {
		pred = head;
		curr = getReference(pred->next.load());

		while (true) {
			succ = get(curr->next.load(), &marked);
			while (marked) {
				uintptr_t old = MarkableReference(curr, marked);
				uintptr_t altered = MarkableReference(succ, false);
				if (!pred->next.compare_exchange_strong(old, altered))
					goto retry;

				curr = succ;
				succ = get(curr->next.load(), &marked);
			}

			if (curr->key >= key) {
				return;
			}
			pred = curr;
			curr = succ;
		}
	}
}

class Pool {
public:
	unsigned char bits[THREAD_COUNT + 1][NUM_TRANSACTIONS / THREAD_COUNT];
	Desc *descriptors[THREAD_COUNT + 1][NUM_TRANSACTIONS / THREAD_COUNT];
	Node *nodes[THREAD_COUNT + 1][NUM_TRANSACTIONS / THREAD_COUNT][TRANSACTION_SIZE];
	HelpStack helpstack[THREAD_COUNT + 1];
	int counter[THREAD_COUNT] = {};

	// Initialize our random bits and ints
	Pool() {
		srand(time(NULL));
		for (int i = 0; i < THREAD_COUNT; i++) {
			for (int j = 0; j < NUM_TRANSACTIONS / THREAD_COUNT; j++) {
				descriptors[i][j] = new Desc;
				descriptors[i][j]->size = TRANSACTION_SIZE;
				descriptors[i][j]->ops = new Operation[descriptors[i][j]->size];
				descriptors[i][j]->status.store(Active);
				for (int k = 0; k < descriptors[i][j]->size; k++) {
					// The following lines will enable probability functionalities
					// for operations to be performed
					int x = rand() % 100;
					OpType random;

					if (x < PERCENT_INSERT) {													// % insert
						random = Insert;
						nodes[i][j][k] = new Node();
					}
					else if (x < PERCENT_DELETE + PERCENT_INSERT) {	// % delete
						random = Delete;
					}
					else {             											 				// % find
						random = Find;
					}

					descriptors[i][j]->ops[k].key = rand() % KEY_RANGE;
					descriptors[i][j]->ops[k].type = random;
				}
			}
		}

		// Prepopulate the list with these
		// Uses an additional "thread" to store descriptors, bits, etc.
		if (NUM_TRANSACTIONS / THREAD_COUNT < KEY_RANGE) {
			cout << "Not enough space in pool." << endl;
			exit(EXIT_FAILURE);
		}
		for (int j = 0; j < KEY_RANGE; j++) {
			int i = THREAD_COUNT;
			descriptors[i][j] = new Desc;
			descriptors[i][j]->size = 1;
			descriptors[i][j]->ops = new Operation[descriptors[i][j]->size];
			descriptors[i][j]->status.store(Active);
			for (int k = 0; k < descriptors[i][j]->size; k++) {
				descriptors[i][j]->ops[k].key = rand() % KEY_RANGE;
				descriptors[i][j]->ops[k].type = Insert;
				nodes[i][j][k] = new Node();
			}
		}
	}
};

Pool pool; // Create our pool

class List {
private:
	Node *head;

public:
	List() {
		head = new Node(INT_MIN, new Node(KEY_RANGE)); // Create our head and tail, which cannot be destroyed
	}

	bool findNode(int key, Desc* desc, int opid, int threadNum, int transactionNum)
	{
		NodeInfo* info = new NodeInfo;
		info->desc = desc;
		info->opid = opid;
		Status ret;
		while (true) {
			Node *pred, *curr;
			find(head, key, pred, curr);
			if (isNodePresent(curr, key))
				ret = updateInfo(curr, info, true, threadNum, transactionNum);
			else
				ret = fail;
			if (ret == success) return true;
			if (ret == fail) return false;
		}
	}

	bool insertNode(int key, Desc* desc, int opid, int threadNum, int transactionNum)
	{

		NodeInfo* info = new NodeInfo;
		info->desc = desc;
		info->opid = opid;
		Status ret;
		while (true) {
			Node *pred, *curr;
			find(head, key, pred, curr);
			if (isNodePresent(curr, key))
				ret = updateInfo(curr, info, false, threadNum, transactionNum);
			else {
				bool insRet = do_insert(key, threadNum, transactionNum, opid);
				if (insRet)
					ret = success;
				else
					ret = fail;
			}
			if (ret == success) return true;
			if (ret == fail) return false;
		}

		//return do_insert(key, threadNum, transactionNum, opid);
	}

	bool deleteNode(int key, Desc* desc, int opid, int threadNum, int transactionNum)
	{
		NodeInfo* info = new NodeInfo;
		info->desc = desc;
		info->opid = opid;
		Status ret;
		while (true) {
			Node *pred, *curr;
			find(head, key, pred, curr);
			if (isNodePresent(curr, key))
				ret = updateInfo(curr, info, true, threadNum, transactionNum);
			else
				ret = fail;
			if (ret == success) {
				//del = curr;
				return true;
			}
			if (ret == fail) {
				//del == NULL;
				return false;
			}
		}

		//do_delete(key, threadNum);
	}

	bool isNodePresent(Node* n, int key) {
		return n->key == key;
	}

	bool isKeyPresent(NodeInfo* info, Desc* desc) {
		OpType op = info->desc->ops[info->opid].type;
		TxStatus status = info->desc->status.load();
		switch (status) {
		case Active:
			if (info->desc == desc)
				return op == Find || op == Insert;
			else
				return op == Find || op == Delete;
		case Committed:
			return op == Find || op == Insert;
		case Aborted:
			return op == Find || op == Delete;
		}
		cout << "Error: isKeyPresent found a key in an invalid state." << endl;
		return false;
	}

	Status updateInfo(Node* n, NodeInfo* info, bool wantKey, int threadNum, int transactionNum) {
		NodeInfo* oldinfo = n->info.load();
		if (isMarked(oldinfo)) {
			do_delete(n->key, threadNum); // TODO: Find a way to call the normal delete here
			return retry;
		}
		if (oldinfo->desc != info->desc) {
			executeOps(oldinfo->desc, oldinfo->opid + 1, threadNum, transactionNum);
		}
		else if (oldinfo->opid >= info->opid) {
			return success;
		}
		bool hasKey = isKeyPresent(oldinfo, oldinfo->desc); // TODO: What descriptor do we pass in isKeyPresent? Should have second argument
		if ((!hasKey && wantKey) || (hasKey && !wantKey)) {
			return fail;
		}
		if (info->desc->status.load() != Active) {
			return fail;
		}
		if (n->info.compare_exchange_strong(oldinfo, info)) {
			return success;
		}
		else {
			return retry;
		}
	}

	bool executeTransaction(Desc* desc, int threadNum, int transactionNum) {
		pool.helpstack[threadNum].init();
		executeOps(desc, 0, threadNum, transactionNum);
		return (desc->status.load() == Committed);
	}

	void executeOps(Desc* desc, int opid, int threadNum, int transactionNum) {

		bool ret = true;
		if (pool.helpstack[threadNum].contains(desc)) {
			TxStatus expected = Active;
			TxStatus changed = Aborted;
			desc->status.compare_exchange_strong(expected, changed);
			return;
		}

		pool.helpstack[threadNum].push(desc);

		while ((desc->status.load() == Active) && ret && (opid < desc->size)) {

			Operation& op = desc->ops[opid];

			switch (op.type) {
			case Insert:
				ret = insertNode(op.key, desc, opid, threadNum, transactionNum);
				break;
			case Delete:
				ret = deleteNode(op.key, desc, opid, threadNum, transactionNum);
				break;
			case Find:
				ret = findNode(op.key, desc, opid, threadNum, transactionNum);
				break;
			};

			opid++;
		}

		pool.helpstack[threadNum].pop();

		if (ret) {
			TxStatus expected = Active;
			TxStatus changed = Committed;
			if (desc->status.compare_exchange_strong(expected, changed)) {
				//markDelete();
			}
		}
		else {
			TxStatus expected = Active;
			TxStatus changed = Aborted;
			desc->status.compare_exchange_strong(expected, changed);
		}
	}



	bool do_insert(int num, int threadNum, int transactionNum, int opid) {
		int key = num;

		while (true) {
			Node *pred;
			Node *curr;
			find(head, key, pred, curr);

			if (curr->key == key) {
				return false;
			}
			else {
				Node *node = getNode2(threadNum, transactionNum, opid, num, curr);
				uintptr_t old = MarkableReference(curr, false);
				uintptr_t altered = MarkableReference(node, false);
				if (pred->next.compare_exchange_strong(old, altered))
					return true;
			}
		}
	}

	bool do_delete(int num, int threadNum) {
		int key = num;

		while (true) {
			Node *pred, *curr;
			find(head, key, pred, curr);

			if (curr->key != key) {
				return false;
			}
			else {
				Node *succ = getReference(curr->next.load());
				uintptr_t old = MarkableReference(succ, false);
				uintptr_t altered = MarkableReference(succ, true);

				if (curr->next.compare_exchange_strong(old, altered)) {
					uintptr_t old2 = MarkableReference(curr, false);
					uintptr_t altered2 = MarkableReference(succ, false);
					pred->next.compare_exchange_strong(old2, altered2);
					return true;
				}
			}
		}
	}

	Node* do_locatePred(int num) {
		int key = num;
		bool marked = false;
		Node *curr = head;
		Node *succ = getReference(head->next.load());

		while (curr->key < key) {
			curr = getReference(curr->next.load());
			succ = get(curr->next.load(), &marked);
		}

		if (curr->key == key && !marked)
			return curr;

		return succ;
	}
};

List list; // Create our list

		   /*
		   static Node* getNode(int threadNum, int operationNum, int num, Node *succ) {
		   Node *node = pool.nodes[threadNum][operationNum];
		   node->item = num;
		   node->key = num;
		   node->next.store(MarkableReference(succ));
		   return node;
		   }*/

static Node* getNode2(int threadNum, int transactionNum, int opid, int num, Node *succ) {
	Node *node = pool.nodes[threadNum][transactionNum][opid];
	node->item = num;
	node->key = num;
	node->next.store(MarkableReference(succ));
	return node;
}

void runThread(int threadNum) {


	for (int i = 0; i < NUM_TRANSACTIONS / THREAD_COUNT; i++) {
		list.executeTransaction(pool.descriptors[threadNum][i], threadNum, i);
		if (pool.descriptors[threadNum][i]->status.load() == Committed) {
			pool.counter[threadNum] += 1;
		}
		if (DEBUG) cout << pool.descriptors[threadNum][i]->status.load();
	}


	/*
	for(int i=0; i<NUM_TRANSACTIONS/THREAD_COUNT; i++) {
	switch(pool.bits[threadNum][i]) {
	case 0:
	if(list.insertNode(pool.ints[threadNum][i], threadNum, i)) {
	if(DEBUG)  cout << "Inserted " << pool.ints[threadNum][i] << endl;
	} else {
	if(DEBUG) cout << "Failed to insert " << pool.ints[threadNum][i] << " (Already in the set)" << endl;
	}
	break;
	case 1:
	if(list.deleteNode(pool.ints[threadNum][i], threadNum)) {
	if(DEBUG) cout << "Removed" << pool.ints[threadNum][i] << endl;
	} else {
	if(DEBUG) cout << "Failed to remove " << pool.ints[threadNum][i] << " (Not in the set)" << endl;
	}
	break;
	case 2:
	if(list.findNode(pool.ints[threadNum][i], threadNum) ){
	if(DEBUG) cout << "Found" << pool.ints[threadNum][i] << endl;
	} else {
	if(DEBUG) cout << "Did not find " << pool.ints[threadNum][i] << endl;
	}
	break;
	default: // This should never be possible, but we need to handle it anyway
	if(DEBUG) cout << "Invalid case set by bit pool (" <<  (int)pool.bits[threadNum][i] << ")" << endl;
	break;
	}
	}
	*/
}

int main(int argc, char *argv[]) {
	// Prepopulate the list
	for (int i = 0; i < KEY_RANGE; i++) {
		list.executeTransaction(pool.descriptors[THREAD_COUNT][i], THREAD_COUNT, i);
		if (DEBUG) cout << pool.descriptors[THREAD_COUNT][i]->status.load();
	}

	thread threads[THREAD_COUNT]; // Create our threads

	auto start = chrono::system_clock::now(); // Get the time
	for (long i = 0; i < THREAD_COUNT; i++) { // Start our threads
		threads[i] = thread(runThread, i);
	}
	for (int i = 0; i < THREAD_COUNT; i++) { // Wait for all threads to complete
		threads[i].join();
	}
	auto total = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start); // Get total execution time

	int comitted = 0;
	for (int i = 0; i < THREAD_COUNT; i++) {
		comitted += pool.counter[i]; // Tally all comitted operations
	}

	double throughput = comitted / (total.count() * 1000);

	cout << "Ran with " << THREAD_COUNT << " threads and " << TRANSACTION_SIZE << " operations per transaction" << endl;
	cout << total.count() << " milliseconds" << endl;
	cout << comitted << " out of " << NUM_TRANSACTIONS << " successful commits" << endl;

	system("pause");

	return 0;
}
