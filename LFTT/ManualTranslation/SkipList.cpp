#include "SkipList.h"

// Pointer Marking
#define SET_MARK(_p)	(Node*)(((uintptr_t)(_p)) | 1)
#define CLR_MARK(_p)	(Node*)(((uintptr_t)(_p)) & ~1)
#define IS_MARKED(_p)	(Node*)(((uintptr_t)(_p)) & 1)

Node *head, *tail;

int randomLevel() {
	return rand() % MAX_LEVEL;
}

void init() {
	head = new Node;
	head->key = INT_MIN;
	tail = new Node;
	tail->key = INT_MAX;
	for (int i = 0; i < MAX_LEVEL + 1; i++) {
		head->next[i].store((uintptr_t)tail);
		tail->next[i].store((uintptr_t)nullptr);
	}
	srand(time(nullptr));
}
bool Do_LocatePred(int key, Node **preds, Node **succs) {
	int bottomLevel = 0;
	bool marked = false;
	bool snip;
	Node *pred = nullptr;
	Node *curr = nullptr;
	Node *succ = nullptr;
retry:
	pred = head;
	for (int level = MAX_LEVEL; level >= bottomLevel; level--) {
		curr = (Node*)pred->next[level].load();
		while (true) {
			succ = (Node*)curr->next[level].load();
			marked = IS_MARKED(succ);
			succ = CLR_MARK(succ);

			while (marked) {
				uintptr_t oldNode = (uintptr_t)CLR_MARK(curr);
				uintptr_t newNode = (uintptr_t)CLR_MARK(succ);
				snip = pred->next[level].compare_exchange_strong(oldNode, newNode);

				if (!snip) {
					goto retry;
				}
				curr = (Node*)pred->next[level].load();
				succ = (Node*)curr->next[level].load();
				marked = IS_MARKED(succ);
				succ = CLR_MARK(succ);
			}
			if (curr->key < key) {
				pred = curr;
				curr = succ;
			}
			else {
				break;
			}
		}
		preds[level] = pred;
		succs[level] = curr;
	}
	return (curr->key == key);
}
Node* Do_LocatePred(int key) {
	Node *preds[MAX_LEVEL + 1];
	Node *succs[MAX_LEVEL + 1];
	bool located = Do_LocatePred(key, preds, succs);
	return succs[0]; // TODO: Is this correct? 
}
bool Do_Insert(Node* n) {
	int topLevel = randomLevel();
	int bottomLevel = 0;
	Node *preds[MAX_LEVEL + 1];
	Node *succs[MAX_LEVEL + 1];
	while (true) {
		bool found = Do_LocatePred(n->key, preds, succs);
		if (found) {
			// Return false if the node already exists in the list.
			return false;
		}
		else {
			Node *newNode = n;
			for (int level = bottomLevel; level <= topLevel; level++) {
				Node *succ = succs[level];
				n->next[level].store((uintptr_t)CLR_MARK(succ));
			}
			Node *pred = preds[bottomLevel];
			Node *succ = succs[bottomLevel];
			newNode->next[bottomLevel].store((uintptr_t)CLR_MARK(succ));
			uintptr_t oldNodePtr = (uintptr_t)CLR_MARK(succ);
			uintptr_t newNodePtr = (uintptr_t)CLR_MARK(newNode);
			// Atomic step where node is physicially included.
			if (!pred->next[bottomLevel].compare_exchange_strong(oldNodePtr, newNodePtr)) {
				continue;
			}
			// Adjust the pointers for the remaining levels.
			for (int level = bottomLevel + 1; level <= topLevel; level++) {
				while (true) {
					pred = preds[level];
					succ = succs[level];
					uintptr_t oldNodePtr = (uintptr_t)CLR_MARK(succ);
					uintptr_t newNodePtr = (uintptr_t)CLR_MARK(newNode);
					if (pred->next[level].compare_exchange_strong(oldNodePtr, newNodePtr)) {
						break;
					}
					// Update our preds and succs.
					Do_LocatePred(n->key, preds, succs);
				}
			}
			// Return true when finished.
			return true;
		}
	}
}
bool Do_Delete(Node* n) {
	int bottomLevel = 0;
	Node *preds[MAX_LEVEL + 1];
	Node *succs[MAX_LEVEL + 1];
	Node *succ;
	while (true) {
		bool found = Do_LocatePred(n->key, preds, succs);
		if (!found) {
			// If the node does not physically exist, there is nothing that needs to be done.
			return false;
		}
		else {
			// Get the node along the botom level, since the bottom level is the only one that ensures atomicity.
			Node *nodeToRemove = succs[bottomLevel];
			// Go through all of the other levels from the top down.
			for (int level = nodeToRemove->topLevel; level >= bottomLevel + 1; level--) {
				bool marked = false;
				// Get the successor of the node we want to delete.
				succ = (Node*)nodeToRemove->next[level].load();
				marked = IS_MARKED(succ);
				succ = CLR_MARK(succ);
				// If the the node to remove does not have a mark in its pointer, mark it. If we fail to physically remove it, we can come back later.
				// Keep attempting to mark until we succeed.
				while (!marked) {
					// Attempt mark.
					nodeToRemove->next[level].store((uintptr_t)SET_MARK(succ));
					// Retrieve mark and update if we succeeded. 
					succ = (Node*)nodeToRemove->next[level].load();
					marked = IS_MARKED(succ);
					succ = CLR_MARK(succ);
				}
			}
			bool marked = false;
			succ = (Node*)nodeToRemove->next[bottomLevel].load();
			marked = IS_MARKED(succ);
			succ = CLR_MARK(succ);
			while (true) {
				// Attempt to mark the node to remove for deletion.
				uintptr_t oldNode = (uintptr_t)CLR_MARK(succ);
				uintptr_t newNode = (uintptr_t)SET_MARK(succ);
				bool iMarkedIt = nodeToRemove->next[bottomLevel].compare_exchange_strong(oldNode, newNode);
				succ = (Node*)succs[bottomLevel]->next[bottomLevel].load();
				marked = IS_MARKED(succ);
				succ = CLR_MARK(succ);
				if (iMarkedIt) {
					Do_LocatePred(n->key, preds, succs);
					return true;
				}
				else if (marked) {
					return false;
				}
			}
		}
	}
}
bool Do_Find(int key) {
	int bottomLevel = 0;
	int v = key;
	bool marked = false;
	Node *pred = head;
	Node *curr = nullptr;
	Node *succ = nullptr;
	for (int level = MAX_LEVEL; level >= bottomLevel; level--) {
		curr = (Node*)pred->next[level].load();
		while (true) {
			succ = (Node*)curr->next[level].load();
			marked = IS_MARKED(succ);
			succ = CLR_MARK(succ);

			while (marked) {
				curr = (Node*)pred->next[level].load();
				succ = (Node*)curr->next[level].load();
				marked = IS_MARKED(succ);
				succ = CLR_MARK(succ);
			}
			if (curr->key < v) {
				pred = curr;
				curr = succ;
			}
			else {
				break;
			}
		}
	}
	return (curr->key == v);
}