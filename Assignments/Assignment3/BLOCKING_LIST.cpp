#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <climits>
#include <cstdlib>
using namespace std;

// Following three are adjustable to run the program
// with different sets of functionality
const int THREAD_COUNT = 64; 
const int NUM_OPERATIONS = 200000;
const bool DEBUG = false;

class Node;
class Pool;
class List;

static Node* getNode(int, int, int, Node*);

class Node {
	public:
		int item;
		int key;
		Node *next;

		Node() {
			item = INT_MIN+1;
			key = item;
			next = NULL;
		}
		Node(int num) {
			item = num;
			key = num;
			next = NULL;
		}
		Node(int num, Node *succ) {
			item = num;
			key = num%INT_MAX;
			next = succ;
		}
};

class Pool {
	public:
		unsigned char bits[THREAD_COUNT][NUM_OPERATIONS/THREAD_COUNT];
		int ints[THREAD_COUNT][NUM_OPERATIONS / THREAD_COUNT];
		Node *nodes[THREAD_COUNT][NUM_OPERATIONS / THREAD_COUNT];

		// Initialize our random bits and ints
		Pool() {
			srand(time(NULL));
			for(int i=0; i<THREAD_COUNT; i++) {
				for(int j=0; j<NUM_OPERATIONS/THREAD_COUNT; j++) {

					// The following commented lines will enable probability functionalities
					// for operations to be performed.
					// double val = (double)rand() / 3;		
					// int random;

					// if (val < 0.15)		15% insert
					// 	random = 0;
					// else if (val < 0.2)	5% 	delete
					// 	random = 1;
					// else					80% find
					// 	random = 2; 

					bits[i][j] = (unsigned char)rand()%3; // 0=insert,1=delete,2=find
					if(bits[i][j] == 0)
						nodes[i][j] = new Node();
					ints[i][j] = rand()%INT_MAX;
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

		bool add(int num, int threadNum, int operationNum) {
			Node *pred, *curr;
			int key = num;
			lock.lock();
			pred = head;
			curr = pred->next;
			while(curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			if(key == curr->key) {
				lock.unlock();
				return false;
			} else {
				Node *node = getNode(threadNum, operationNum, num, curr);
				lock.unlock();
				return true;
			}
		}

		bool remove(int num, int threadNum) {
			Node *pred, *curr;
			int key = num;
			lock.lock();
			pred = head;
			curr = pred->next;

			while(curr->key < key) {
				pred = curr;
				curr = curr->next;
			}

			if(key == curr->key) {
				pred->next = curr->next;
				lock.unlock();
				return true;
			} else {
				lock.unlock();
				return false;
			}
		}

		bool contains(int num) {
			Node *pred, *curr;
			int key = num;
			lock.lock();
			pred = head;
			curr = pred->next;
			while(curr->key < key) {
				pred = curr;
				curr = curr->next;
			}
			if(key == curr->key) {
				lock.unlock();
				return true;
			} else {
				lock.unlock();
				return false;
			}
		}
};

List list; // Create our list
Pool pool; // Create our pool

static Node* getNode(int threadNum, int operationNum, int num, Node *succ) {
	Node *node = pool.nodes[threadNum][operationNum];
	node->item = num;
	node->key = num;
	node->next = succ;
	return node;
}

void runThread(int threadNum) {
	for(int i=0; i<NUM_OPERATIONS/THREAD_COUNT; i++) {
		switch(pool.bits[threadNum][i]) {
			case 0:
				if(list.add(pool.ints[threadNum][i], threadNum, i)) {
					if(DEBUG) cout << "Inserted " << pool.ints[threadNum][i] << endl;
				} else {
					if(DEBUG) cout << "Failed to insert " << pool.ints[threadNum][i] << " (Already in the set)" << endl;
				}
				break;
			case 1:
				if(list.remove(pool.ints[threadNum][i], threadNum)) {
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