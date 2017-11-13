#include "LogicalStatus.h"
using namespace std;

// Algorithm 3: Logical Status
bool IsNodePresent(Node* n, int key) {
	assert(n != nullptr);
	return n->key == key;
}
bool IsKeyPresent(Node* n) {
	NodeInfo* info = n->info;
	Desc* desc = info->desc;
	OpType op = info->desc->ops[info->opid].type;
	TxStatus status = info->desc->status.load();
	switch (status) {
	case Active:
		if (info->desc == desc) {
			return op == Find || op == Insert;
		}
		else {
			return op == Find || op == Delete;
		}
	case Committed:
		return op == Find || op == Insert;
	case Aborted:
		return op == Find || op == Delete;
	}
	return false;
}