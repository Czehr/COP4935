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
const int THREAD_COUNT = 2;
const int TRANSACTION_COUNT = 50000;
const int NUM_OPERATIONS = 20000;
const bool DEBUG = false;

class MarkableReference;
class Node;
class Pool;
class Window;
class List;

static Node* getNode(int, int, int, Node*);
static Node* getNode2(int, int, int, int, Node*);
static Window* getWindow(Node*, Node*, int);

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
			item = INT_MIN+1;
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

class Window {
    public:
        Node *pred, *curr;

		Window() {
			pred = NULL;
			curr = NULL;
		}

        Window(Node *myPred, Node *myCurr) {
            pred = myPred;
            curr = myCurr;
        }

        static Window *find(Node *head, int key, int threadNum) {
            Node *pred, *curr, *succ;
            bool marked = false;

            retry: while(true) {
                pred = head;
                curr = getReference(pred->next.load());

                while(true) {
                    succ = get(curr->next.load(), &marked);
                    while(marked) {
                        uintptr_t old = MarkableReference(curr, marked);
                        uintptr_t altered = MarkableReference(succ, false);
                        if(!pred->next.compare_exchange_strong(old, altered))
                            goto retry;

                        curr = succ;
                        succ = get(curr->next.load(), &marked);
                    }

                    if(curr->key >= key)
                        return getWindow(pred, curr, threadNum);
                    pred = curr;
                    curr = succ;
                } 
            }
        }
};

class Pool {
	public:
		unsigned char bits[THREAD_COUNT][NUM_OPERATIONS/THREAD_COUNT];
		int ints[THREAD_COUNT][NUM_OPERATIONS/THREAD_COUNT];
		Node *nodes[THREAD_COUNT][NUM_OPERATIONS/THREAD_COUNT];
		Node *transactionNodes[THREAD_COUNT][TRANSACTION_COUNT][OPERATIONS_PER_TRANSACTION];
		Desc *transactions[THREAD_COUNT][TRANSACTION_COUNT/THREAD_COUNT];
		Window *windows[THREAD_COUNT];

		// Initialize our random bits and ints
		Pool() {
			srand(time(NULL));
			for(int i=0; i<THREAD_COUNT; i++) {
				windows[i] = new Window();
				for(int j=0; j<NUM_OPERATIONS/THREAD_COUNT; j++) {

					// The following commented lines will enable probability functionalities
					// for operations to be performed 
					 int x = rand() % 100;
					 int random;

					 if (x < 15)            // 15% insert
					 	random = 0;
					 else if (x < 20)       // 5% 	delete
					 	random = 1;
					 else                   //	80% find
					 	random = 2;

					bits[i][j] = (unsigned char)random;// 0=insert,1=delete,2=find
					if(bits[i][j] == 0)
						nodes[i][j] = new Node();
					ints[i][j] = rand()%INT_MAX;
				}

				//Transaction Description creation
				for(int j=0; j < TRANSACTION_COUNT/THREAD_COUNT; j++){

				
					transactions[i][j] = new Desc();
					transactions[i][j]->size = OPERATIONS_PER_TRANSACTION;
					transactions[i][j]->status = Active;
					
					
					for(int k=0; k < OPERATIONS_PER_TRANSACTION; k++){
						
						int randomNumber = rand() % 15;
						int newRandom = rand() % 100;

						if(randomNumber < 15){
							transactions[i][j]->ops[k].type = Insert;
							transactions[i][j]->ops[k].key = newRandom;
							transactionNodes[i][j][k] = new Node(newRandom);
						}else if (randomNumber < 20){	
								transactions[i][j]->ops[k].type = Delete;
			

								transactions[i][j]->ops[k].key = 1;
						}else{
							
								transactions[i][j]->ops[k].type = Find;
		

								transactions[i][j]->ops[k].key = 2;
						}
						
					}
				} 
			}
		}
};

class List {
	private:
		Node *head;
		HelpStack helpstack;


	public:
		List() {
			head = new Node(INT_MIN, new Node(INT_MAX)); // Create our head and tail, which cannot be destroyed
		}

		bool findNode(int key, Desc* desc, int opid, int threadNum, int transactionNum)
		{
			NodeInfo* info = new NodeInfo;
			info->desc = desc;
			info->opid = opid;
			Status ret;
			while(true){
				Window *window = Window::find(head, key, threadNum);
				Node* curr = window->pred; //do_locatePred(key);
				if(isNodePresent(curr, key))
					ret = updateInfo(curr, info, true, threadNum, transactionNum);
				else
					ret = fail;
				if(ret == success) return true;
				if(ret == fail) return false;
			}



			//Window *window = Window::find(head, key, threadNum);
			//return window->curr;
		}

		bool insertNode(int key, Desc* desc, int opid, int threadNum, int transactionNum)
		{
			
			NodeInfo* info = new NodeInfo;
			info->desc = desc;
			info->opid = opid;
			Status ret;
			while(true){
				Window *window = Window::find(head, key, threadNum);
				Node* curr = window->pred;
				if(isNodePresent(curr, key))
					ret = updateInfo(curr, info, false, threadNum, transactionNum);
				else{
					bool insRet = do_insert(key, threadNum, transactionNum, opid);
					if(insRet)
						ret = success;
					else
						ret = fail;
				}
				if(ret == success) return true;
				if(ret == fail) return false;
			}

			//return do_insert(key, threadNum, transactionNum, opid);
		}

		bool deleteNode(int key, Desc* desc, int opid, int threadNum, int transactionNum)
		{
			NodeInfo* info = new NodeInfo;
			info->desc = desc;
			info->opid = opid;
			Status ret;
			while(true){
				Window *window = Window::find(head, key, threadNum);
				Node* curr = window->pred;
				if (isNodePresent(curr, key))
					ret = updateInfo(curr, info, true, threadNum, transactionNum);
				else
					ret = fail;
				if (ret == success){
					//del = curr;
					return true;
				}
				if(ret == fail){
					//del == NULL;
					return false;
				}
			}

			//do_delete(key, threadnum);
		}

		bool isNodePresent(Node* n, int key) {
			return n->key == key;
		}

		bool isKeyPresent(NodeInfo* info, Desc* desc) {
		OpType op = info->desc->ops[info->opid].type;
		TxStatus status = info->desc->status;
		switch(status) {
			case Active:
				if(info->desc==desc)
					return op == Find || op == Insert;
				else
					return op == Find || op == Delete;
			case Comitted:
				return op == Find || op == Insert;
			case Aborted:
				return op == Find || op == Delete;
			}
		}

		Status updateInfo(Node* n, NodeInfo* info, bool wantKey, int threadnum, int transactionNum) {
			NodeInfo* oldinfo = n->info.load();
			if(isMarked(oldinfo)) {
				do_delete(n->key, threadnum); // TODO: Find a way to call the normal delete here
				return retry;
			}
			if(oldinfo->desc != info->desc) {
				executeOps(oldinfo->desc, oldinfo->opid + 1, threadnum, transactionNum);
			} else if(oldinfo->opid >= info->opid) {
				return success;
			}
			bool hasKey = isKeyPresent(oldinfo, oldinfo->desc); // TODO: What descriptor do we pass in isKeyPresent? Should have second argument
			if((!hasKey && wantKey) || (hasKey && !wantKey)) {
				return fail;
			}
			if(info->desc->status != Active) {
				return fail;
			}
			if(n->info.compare_exchange_strong(oldinfo, info)) {
				return success;
			} else {
				return retry;
			}
		}

		bool executeTransaction(Desc* desc, int threadnum, int transactionNum){
			helpstack.init();
			executeOps(desc, 0, threadnum, transactionNum);
			return (desc->status == Comitted);
		}

		void executeOps(Desc* desc, int opid, int threadnum, int transactionNum){

			 bool ret = true;
			 if(helpstack.contains(desc)){
			 	//compare and swap
			 	return;
			 }

			 //helpstack.push(desc);
			 
			 while((desc->status == Active) && ret && (opid < desc->size)){

			 	

			 	Operation& op = desc->ops[opid];
			
			 	switch (op.type){
			 		case Insert:
			 			ret = insertNode(op.key, desc, opid, threadnum, transactionNum);
			 			break;
			 		case Delete:
			 			ret = deleteNode(op.key, desc, opid, threadnum, transactionNum);
			 			break;
			 		case Find:
			 			ret = findNode(op.key, desc, opid, threadnum, transactionNum);
			 			break;

			 	};
			 	
			 	opid++;
			 }

			 //helpstack.pop();

			 //if(ret){
			 	//if (compare and swap Committed)
			 		//markDelete();
			//}
			 //else
			 	//compare and swap Aborted

		}



		bool do_insert(int num, int threadNum, int transactionNum, int opid) {
            int key = num;

            while(true) {
                Window *window = Window::find(head, key, threadNum);
                Node *pred = window->pred, *curr = window->curr;

                if(curr->key == key) {
                    return false;
                } else {
                    Node *node = getNode2(threadNum, transactionNum, opid, num, curr);
                    uintptr_t old = MarkableReference(curr, false);
                    uintptr_t altered = MarkableReference(node, false);
                    if(pred->next.compare_exchange_strong(old, altered))
                        return true;
                }
            }
		}

		bool do_delete(int num, int threadNum) {
            int key = num;

            while(true) {
                Window *window = Window::find(head, key, threadNum);
                Node *pred = window->pred, *curr = window->curr;

                if(curr->key != key) {
                    return false;
                } else {
                    Node *succ = getReference(curr->next.load());
                    uintptr_t old = MarkableReference(succ, false);
                    uintptr_t altered = MarkableReference(succ, true);

                    if(curr->next.compare_exchange_strong(old, altered)) {
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

            while(curr->key < key) {
                curr = getReference(curr->next.load());
                Node *succ = get(curr->next.load(), &marked);
            }

            if (curr->key == key && !marked)
            	return curr;
		}
};

List list; // Create our list
Pool pool; // Create our pool

/*
static Node* getNode(int threadNum, int operationNum, int num, Node *succ) {
	Node *node = pool.nodes[threadNum][operationNum];
	node->item = num;
	node->key = num;
	node->next.store(MarkableReference(succ));
	return node;
}*/

static Node* getNode2(int threadNum, int transactionNum, int opid, int num, Node *succ) {
	Node *node = pool.transactionNodes[threadNum][transactionNum][opid];
	node->item = num;
	node->key = num;
	node->next.store(MarkableReference(succ));
	return node;
}

static Window* getWindow(Node *myPred, Node *myCurr, int threadNum) {
	Window *window = pool.windows[threadNum];
	window->pred = myPred;
	window->curr = myCurr;
	return window;
}

void runThread(int threadNum) {


	for(int i = 0; i < TRANSACTION_COUNT/THREAD_COUNT; i++){
		list.executeTransaction(pool.transactions[threadNum][i], threadNum, i);
	}


	/*
	for(int i=0; i<NUM_OPERATIONS/THREAD_COUNT; i++) {
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

	thread threads[THREAD_COUNT]; // Create our threads
	auto start = chrono::system_clock::now(); // Get the time
	for(long i=0; i<THREAD_COUNT; i++) { // Start our threads
		threads[i] = thread(runThread, i);
	}
	for(int i=0; i<THREAD_COUNT; i++) { // Wait for all threads to complete
		threads[i].join();
	}
	auto total = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start); // Get total execution time
	cout << "Total runtime is " << total.count() << " milliseconds" << endl;
	
	return 0;
}
