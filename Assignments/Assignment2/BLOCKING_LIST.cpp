#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <climits>
#include <cstdlib>
using namespace std;

const int THREAD_COUNT = 4;
const int NUM_OPERATIONS = 33554432; // Total number of operations performed
const bool DEBUG = true;

class Bits {
	public:
		int bits[THREAD_COUNT][NUM_OPERATIONS/THREAD_COUNT][2];
		Bits() { // Initialize our random bits
			srand(time(NULL));
			for(int i=0; i<THREAD_COUNT; i++) {
				for(int j=0; j<NUM_OPERATIONS/THREAD_COUNT; j++) {
					bits[i][j][0] = rand()%3; // 0=insert,1=delete,2=find
					bits[i][j][1] = rand()%INT_MAX+INT_MIN; // A random int
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
			key = num%INT_MAX;
			next = NULL;
		}
		Node(int num, Node *succ) {
			item = num;
			key = num%INT_MAX;
			next = succ;
		}
		//T item; // TODO: Try to make this generic
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
			Node *tail = new Node(INT_MAX);
			head = new Node(INT_MIN, tail); // Create our head and tail, which cannot be destroyed
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
		int remove(int num) {
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
		int contains(int num) {
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
Bits bits; // Create our bits

void runThread(int threadNum) {
	for(int i=0; i<NUM_OPERATIONS/THREAD_COUNT; i++) {
		switch(bits.bits[threadNum][i][0]) {
			case 0:
				list.add(bits.bits[threadNum][i][1]);
			case 1:
				list.remove(bits.bits[threadNum][i][1]);
			case 2:
				list.contains(bits.bits[threadNum][i][1]);
			default:
				// This should never be possible, but we need to handle it anyway
				break;
		}
	}
}

int main(int argc, char *argv[]) {
	// Create our threads
	thread threads[THREAD_COUNT];
	
	// Get the time
	auto start = chrono::system_clock::now();
	
	// Start our threads
	for(long i=0; i<THREAD_COUNT; i++) {
		threads[i] = thread(runThread, i);
	}
	// Wait for all threads to complete
	for(int i=0; i<THREAD_COUNT; i++) {
		threads[i].join();
	}
	
	// Get the time
	auto end = chrono::system_clock::now();
	
	// Get total execution time
	auto total = chrono::duration_cast<chrono::milliseconds>(end - start);
	
	cout << "Total runtime is " << total.count() << " milliseconds" << endl;

	return 0;
}