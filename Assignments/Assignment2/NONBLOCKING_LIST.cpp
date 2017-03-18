#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <climits>
#include <cstdlib>
using namespace std;


const int THREAD_COUNT = 4;
const int NUM_OPERATIONS = 20; //33554432; // Total number of operations performed
const bool DEBUG = true;
class MarkableReference;
class Node;
class Pool;
class Window;
class List;

class Pool {
	public:
		unsigned char bits[THREAD_COUNT][NUM_OPERATIONS/THREAD_COUNT];
		int ints[THREAD_COUNT][NUM_OPERATIONS/THREAD_COUNT];
		Pool() { // Initialize our random bits and ints
			srand(time(NULL));
			for(int i=0; i<THREAD_COUNT; i++) {
				for(int j=0; j<NUM_OPERATIONS/THREAD_COUNT; j++) {
					bits[i][j] = (unsigned char)rand()%3; // 0=insert,1=delete,2=find
					ints[i][j] = rand()%INT_MAX; // A random int
				}
			}
		}
};

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

class Node {
	public:
		Node() {
			item = INT_MIN+1;
			key = item;
		}
		Node(int num) {
			item = num;
			key = num%INT_MAX;
		}
		Node(int num, Node *succ) {
			item = num;
			key = num%INT_MAX;
			next.store(MarkableReference(succ));
		}
		//T item; // TODO: Try to make this generic if we have the time
		int item; // Use this int in the meantime
		int key;
		atomic<uintptr_t> next;
};

class Window {
    public:
        Node *pred, *curr;

        Window(Node *myPred, Node *myCurr) {
            pred = myPred;
            curr = myCurr;
        }  

        static Window *find(Node *head, int key) {
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
                        delete curr;
                        curr = succ;
                        succ = get(curr->next.load(), &marked);
                    }

                    if(curr->key >= key)
                        return new Window(pred, curr);
                    pred = curr;
                    curr = succ;
                } 
            }
        }
};

class List {
	private:
		Node *head;
		mutex lock;
	public:
		List() {
			head = new Node(INT_MIN, new Node(INT_MAX)); // Create our head and tail, which cannot be destroyed
		}
		bool add(int num) {
            int key = num;

            while(true) {
                Window *window = Window::find(head, key);
                Node *pred = window->pred, *curr = window->curr;
								delete window;

                if(curr->key == key) {
                    return false;
                } else {
                    Node *node = new Node(num, curr);
                    uintptr_t old = MarkableReference(curr, false);
                    uintptr_t altered = MarkableReference(node, false);
                    if(pred->next.compare_exchange_strong(old, altered))
                        return true;
                    else
                        delete node;
                }
            }
		}
		bool remove(int num) {
            int key = num;

            while(true) {
                Window *window = Window::find(head, key);
                Node *pred = window->pred, *curr = window->curr;
								delete window;

                if(curr->key != key) {
                    return false;
                } else {
                    Node *succ = getReference(curr->next.load());
                    uintptr_t old = MarkableReference(succ, false);
                    uintptr_t altered = MarkableReference(succ, true);

                    if(curr->next.compare_exchange_strong(old, altered)) {
                        uintptr_t old2 = MarkableReference(curr, false);
                        uintptr_t altered2 = MarkableReference(succ, false);

                        if(pred->next.compare_exchange_strong(old2, altered2)) {
                            delete curr;
                        }

                        return true;
                    }
                }
            }
		}

		bool contains(int num) {
            int key = num;
            bool marked = false;
            Node *curr = head;

            while(curr->key < key) {
                curr = getReference(curr->next.load());
                Node *succ = get(curr->next.load(), &marked);
            }

            return (curr->key == key && !marked);
		}
};

List list; // Create our list
Pool pool; // Create our pool

void runThread(int threadNum) {
	for(int i=0; i<NUM_OPERATIONS/THREAD_COUNT; i++) {
		switch(pool.bits[threadNum][i]) {
			case 0:
				if(list.add(pool.ints[threadNum][i])) {
					if(DEBUG)  cout << "Inserted " << pool.ints[threadNum][i] << endl;
				} else {
					if(DEBUG) cout << "Failed to insert " << pool.ints[threadNum][i] << " (Already in the set)" << endl;
				}
				break;
			case 1:
				if(list.remove(pool.ints[threadNum][i])) {
					if(DEBUG) cout << "Removed" << pool.ints[threadNum][i] << endl;
				} else {
					if(DEBUG) cout << "Failed to remove " << pool.ints[threadNum][i] << " (Not in the set)" << endl;
				}
				break;
			case 2:
				if(list.contains(pool.ints[threadNum][i])) {
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