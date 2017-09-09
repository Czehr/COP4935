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

void init() {

}
Node* Do_LocatePred(int k[], locatePredData &data) {
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

}
bool Do_Insert(Node* node) {
	// At this point, the key and NodeDesc should already be set by LFTT.
	locatePredData data;

}
bool Do_Delete(Node* n) {

}
bool Do_Find(int key) {
	locatePredData data;
	Do_LocatePred(KeyToCoord(key), data);
	if (data.dc = D) {
		return data.curr->val;
	}
	else {
		return NULL;
	}
}