#include "Main.h"
using namespace std;

class Pool {
public:
	Desc *descriptors[THREAD_COUNT][NUM_TRANSACTIONS / THREAD_COUNT];

	// Initialize our randomized transaction descriptors.
	Pool() {
		srand(time(NULL));
		for (int i = 0; i < THREAD_COUNT; i++) {
			for (int j = 0; j < NUM_TRANSACTIONS / THREAD_COUNT; j++) {
				descriptors[i][j] = new Desc;
				descriptors[i][j]->status.store(Active);
				for (int k = 0; k < descriptors[i][j]->size; k++) {
					int key = rand() % KEY_RANGE;
					descriptors[i][j]->ops[k].key = key;

					// Use probability distribution of operations.
					int x = rand() % 100;
					OpType random;

					if (x < PERCENT_INSERT) {						// % insert
						random = Insert;
						string *keyString = new string;
						*keyString = to_string(key);
						descriptors[i][j]->ops[k].val = keyString;
					}
					else if (x < PERCENT_DELETE + PERCENT_INSERT) {	// % delete
						random = Delete;
					}
					else {											// % find
						random = Find;
					}

					
					descriptors[i][j]->ops[k].type = random;
				}
			}
		}
	}
};

Pool pool; // Create our pool instance.

void runThread(int threadNum) {
	for (int i = 0; i < NUM_TRANSACTIONS / THREAD_COUNT; i++) {
		ExecuteTransaction(pool.descriptors[threadNum][i]);
		if (DEBUG) cout << pool.descriptors[threadNum][i]->status.load();
	}
}

int main(int argc, char *argv[]) {
	// Initialize the list.
	init();

	// Create our threads.
	thread threads[THREAD_COUNT];

	// Get the current time.
	auto start = chrono::system_clock::now();

	// Start our threads.
	for (long i = 0; i < THREAD_COUNT; i++) {
		threads[i] = thread(runThread, i);
	}

	// Wait for all threads to complete.
	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].join();
	}

	// Get total execution time.
	auto total = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start);

	cout << "Ran with " << THREAD_COUNT << " threads and " << TRANSACTION_SIZE << " operations per transaction" << endl;
	cout << total.count() << " milliseconds" << endl;

	system("pause");

	return 0;
}