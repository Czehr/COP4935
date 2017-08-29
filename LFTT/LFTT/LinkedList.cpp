#include "LinkedList.h"
using namespace std;

// Pointer marking
#define SET_MARK(_p)	(Node*)(((uintptr_t)(_p)) | 1)
#define CLR_MARK(_p)	(Node*)(((uintptr_t)(_p)) & ~1)
#define IS_MARKED(_p)	(Node*)(((uintptr_t)(_p)) & 1)

// The head of our linked list.
Node *head;

void init() {
	// The head and tail are sentinel nodes, so they cannot be destroyed.
	head = new Node;
	head->key = INT_MIN;

	Node *tail = new Node;
	tail->key = INT_MAX;
	tail->next.store(NULL);

	head->next.store((uintptr_t)tail);
}


void Do_LocatePred(Node *&pred, Node *&curr, int key) {
	curr = head;
	Node* pred_next;

	while (curr->key < key)
	{
		pred = curr;
		pred_next = CLR_MARK(pred->next.load());
		curr = pred_next;
		while (IS_MARKED(curr->next.load()))
		{
			curr = CLR_MARK(curr->next.load());
		}
		if (curr != pred_next)
		{
			//Failed to remove deleted nodes, start over from pred.
			unsigned int oldNext = (uintptr_t)pred_next;
			unsigned int newNext = (uintptr_t)curr;
			if (!pred->next.compare_exchange_strong(oldNext, newNext))
			{
				curr = head;
			}
		}
	}
}
// Modified function used in LFTT.
Node* Do_LocatePred(int key) {
	Node *pred = NULL, *curr = NULL;
	// We do not care about the resut of pred, So only return curr.
	Do_LocatePred(pred, curr, key);
	return curr;
}

Status Do_Insert(Node *n) {
	// Pointers retrieved by Do_LocatePred.
	Node *pred = NULL, *curr = NULL;
	// The key for the node we are interested in.
	int key = n->key;
	while (true) {
		Do_LocatePred(pred, curr, key);
		if (curr->key == key) {
			return fail;
		}
		else {
			// Implementation-specific node modification.
			n->next.store((uintptr_t)curr);

			uintptr_t oldNode = (uintptr_t)curr;
			uintptr_t newNode = (uintptr_t)n;
			if (pred->next.compare_exchange_strong(oldNode, newNode))
				return success;
		}
	}
}

Status Do_Delete(Node *n) {
	// Pointers retrieved by Do_LocatePred.
	Node *pred = NULL, *curr = NULL;
	// The key for the node we are interested in.
	int key = n->key;
	while (true) {
		Do_LocatePred(pred, curr, key);
		if (curr->key != key) {
			return fail;
		}
		else {
			uintptr_t succ = (uintptr_t)CLR_MARK(curr->next.load());
			uintptr_t oldNode = (uintptr_t)CLR_MARK(succ);
			uintptr_t newNode = (uintptr_t)SET_MARK(succ);
			if (curr->next.compare_exchange_strong(oldNode, newNode)) {
				// Attempt to perform physical removal. If it fails, it can always be physically removed later.
				uintptr_t oldNode2 = (uintptr_t)CLR_MARK(curr);
				uintptr_t newNode2 = (uintptr_t)CLR_MARK(succ);
				if (pred->next.compare_exchange_strong(oldNode2, newNode2)) {
					// Deallocate unused nodes.
					delete curr; // TODO: Optionally preallocate these and ignore deallocation.
				}
				return success;
			}
		}
	}
}