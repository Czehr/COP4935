#include "MDList.h"
using namespace std;

// Pointer marking
int Fadp = 1, Fdel = 2, Fall = Fadp | Fdel;
#define SET_MARK(_p, _m)	(Node*)(((uintptr_t)(_p)) | _m)
#define CLR_MARK(_p, _m)	(Node*)(((uintptr_t)(_p)) & ~_m)
#define IS_MARKED(_p, _m)	(Node*)(((uintptr_t)(_p)) & _m)

// The head of our multi-dimensional list.
Node *head;

// Make our data easy to pass around, as an alternative to using inline functions.
struct locatePredData {
	Node *pred = NULL;
	Node *curr = head;
	int dp = 0;
	int dc = 0;
	AdoptDesc *ad = NULL;
};

// Mapping function
int *KeyToCoord(int key) {
	// Convert the key to base Dth root of N.
	// Take the restulting key and map each digit to a dimension. 
	int *k = new int[D];
	k[0] = key;
	return k;
}

// Algorithm 6: Child Adoption
void FinishInserting(Node* n, AdoptDesc* ad)
{
	Node *child;
	Node *curr = ad->curr;
	int dp = ad->dp;
	int dc = ad->dc;
	for (int i = dp; i < dc; i++) {

		child = SET_MARK(curr->child[i].load(), Fadp); // TODO: FetchAndOr?
		child = CLR_MARK(child, Fadp);
		if (n->child[i] == nullptr) {
			Node *oldNode = nullptr;
			Node *newNode = child;
			n->child[i].compare_exchange_strong(oldNode, newNode);
		}
	}
	n->adesc.compare_exchange_strong(ad, nullptr);
}

void init() {

}
void Do_LocatePred(int key, locatePredData &data) {
	int *k = KeyToCoord(key);
	while (data.dc < D) {
		while (data.curr != NULL && k[data.dc] > data.curr->k[data.dc]) {
			data.pred = data.curr;
			data.dp = data.dc;
			data.ad = data.curr->adesc;
			if (data.ad != NULL) {
				for (int i = data.ad->dp; i <= data.ad->dc; i++) {
					if (data.dp == i) {
						FinishInserting(data.curr, data.ad);
						break;
					}
				}
			}
			data.curr = CLR_MARK(data.curr->child[data.dc].load(), Fall);
		}
		if (data.curr == NULL || k[data.dc] < data.curr->k[data.dc]) {
			break;
		}
		else {
			data.dc++;
		}
	}
}
Node* Do_LocatePred(int key) {
	return nullptr;
}

// Algorithm 5: Concurrent Insert
bool Do_Insert(Node* node) {
	// At this point, the key and NodeDesc should already be set by LFTT.
	AdoptDesc *ad;
	node->k = KeyToCoord(node->key); // TODO: Put this in the for loop too?
	for (int i = 0; i < D; i++) {
		node->child[i] = nullptr;
	}
	while (true) {
		locatePredData data;
		data.pred = nullptr;
		data.curr = head;
		data.dp = 0;
		data.dc = 0;
		Do_LocatePred(node->key, data);
		ad = (data.curr != nullptr) ? data.curr->adesc.load() : nullptr;
		if (ad != nullptr && data.dp != data.dc) {
			FinishInserting(data.curr, ad);
		}
		if (IS_MARKED(data.pred->child[data.dp].load(), Fdel)) {
			data.curr = SET_MARK(data.curr, Fdel);
			if (data.dc == D - 1) {
				data.dc = D;
			}
		}
		
		// FillNewNode();
		data.ad = nullptr;
		if (data.dp != data.dc) {
			ad = new AdoptDesc;
			data.ad->curr = data.curr;
			data.ad->dp = data.dp;
			data.ad->dc = data.dc;
		}
		for (int i = 0; i < data.dp; i++) {
			node->child[i].store(SET_MARK(node->child[i].load(),Fadp)); //TODO: Is this right?
		}
		for (int i = data.dp; i < D; i++) {
			node->child[i].store(nullptr);
		}
		if (data.dc < D) {
			node->child[data.dc] = data.curr;
		}
		node->adesc = data.ad;
		// End FillNewNode()

		if (data.pred->child[data.dp].compare_exchange_strong(data.curr, node)) {
			if (ad != nullptr) {
				FinishInserting(node, data.ad);
			}
			break;
		}
	}
	
	return true; // TODO: Is this appropriate?
}

// Algorithm 7: Concurrent Delete
bool Do_Delete(Node* n) {
	locatePredData data;
	Node *child = nullptr;
	Node *marked = nullptr;
	while (true)
	{
		data.pred = nullptr;
		data.curr = head;
		data.dp = 0;
		data.dc = 0;
		Do_LocatePred(n->key, data);
		if (data.dc != D) {
			return nullptr;
		}
		marked = SET_MARK(data.curr, Fdel);
		if (data.pred->child[data.dp].compare_exchange_strong(data.curr, marked)) {
			child = marked;
		}
		if (CLR_MARK(child, Fdel | Fadp) == data.curr) {
			if (!IS_MARKED(child, Fdel | Fadp)) {
				return data.curr->val;
			}
			else if (IS_MARKED(child, Fdel)) {
				return nullptr;
			}
		}
	}
}
bool Do_Find(int key) {
	locatePredData data;
	Do_LocatePred(key, data);
	if (data.dc = D) {
		return data.curr->val;
	}
	else {
		return NULL;
	}
}