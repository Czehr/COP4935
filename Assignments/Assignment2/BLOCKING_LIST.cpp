#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <climits>
#include <cstdlib>
using namespace std;

const int THREAD_COUNT = 4;
const int NUM_OPERATIONS = 33554432; // Total number of operations performed
const bool DEBUG = false;

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

class Node {
	public:
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
		//T item; // TODO: Try to make this generic if we have the time
		int item; // Use this int in the meantime
		int key;
		Node *next;
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
			Node *pred, *curr;
			int key = num; // TODO: Number currently hashes to itself. Perhaps this should change
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
				Node *node = new Node(num);
				node->next = curr;
				pred->next = node;
				lock.unlock();
				return true;
			}
		}
		bool remove(int num) {
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
				delete pred->next; // TODO: Is this correct memory management?
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

void runThread(int threadNum) {
	for(int i=0; i<NUM_OPERATIONS/THREAD_COUNT; i++) {
		switch(pool.bits[threadNum][i]) {
			case 0:
				if(list.add(pool.ints[threadNum][i])) {
					if(DEBUG) cout << "Inserted " << pool.ints[threadNum][i] << endl;
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