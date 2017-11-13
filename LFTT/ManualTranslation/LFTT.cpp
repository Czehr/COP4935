#include "LFTT.h"
using namespace std;

// Define our functions in advance.
void *FindNode(int key, Desc* desc, int opid);
bool InsertNode(int key, void *val, Desc* desc, int opid);
bool DeleteNode(int key, Desc* desc, int opid, Node*& del);
void MarkDelete(set<Node*> delnodes, Desc* desc);
void ExecuteOps(Desc* desc, int opid);

// Stack data structure. Used in the helping scheme.
struct HelpStack
{
private:
	int index;
	Desc* descriptors[256]; // TODO: Will this always be enough?
public:
	void init() {
		index = 0;
	}
	void push(Desc* desc) {
		assert(index < 255);
		descriptors[index++] = desc;
	}
	void pop() {
		assert(index > 0);
		index--;
	}
	bool contains(Desc* desc) {
		for (int i = 0; i < index; i++) {
			if (descriptors[i] == desc)
				return true;
		}
		return false;
	}
};

// Algorithm 2: Pointer Marking
#define SET_MARK(_p)	(((uintptr_t)(_p)) | 1)
#define CLR_MARK(_p)	(((uintptr_t)(_p)) & ~1)
#define IS_MARKED(_p)	(((uintptr_t)(_p)) & 1)

// Algorithm 4: Update NodeInfo
Status UpdateNodeInfo(Node* n, NodeInfo* info, bool wantkey) {
	NodeInfo* oldinfo = n->info;
	if (IS_MARKED(oldinfo)) {
		Do_Delete(n);

		return retry;
	}
	if (oldinfo->desc != info->desc) {
		ExecuteOps(oldinfo->desc, oldinfo->opid + 1);
	}
	else if (oldinfo->opid >= info->opid) {
		return success;
	}
	bool haskey = IsKeyPresent(n);
	if ((!haskey && wantkey) || haskey && !wantkey) {
		return fail;
	}
	if (info->desc->status.load() != Active) {
		return fail;
	}
	if (n->info.compare_exchange_strong(oldinfo, info)) {
		return success;
	}
	else {
		return retry;
	}
}

// Algorithm 5: Transaction Execution
thread_local HelpStack helpstack;
bool ExecuteTransaction(Desc* desc) {
	helpstack.init();
	ExecuteOps(desc, 0);
	return desc->status.load() == Committed;
}
void ExecuteOps(Desc* desc, int opid) {
	bool ret = true;
	set<Node*> delnodes;
	if (helpstack.contains(desc)) {
		TxStatus active = Active;
		TxStatus aborted = Aborted;
		desc->status.compare_exchange_strong(active, aborted);
		return;
	}
	helpstack.push(desc);
	while (desc->status.load() == Active && ret && opid < desc->size)
	{
		Operation* op = &desc->ops[opid];
		if (op->type == Find) {
			ret = (FindNode(op->key, desc, opid) != nullptr);
		}
		else if (op->type == Insert) {
			ret = InsertNode(op->key, op->val, desc, opid);
		}
		else if (op->type == Delete) {
			Node* del;
			ret = DeleteNode(op->key, desc, opid, del);
			delnodes.insert(del);
		}
		opid = opid + 1;
	}
	helpstack.pop();
	if (ret == true) {
		TxStatus active = Active;
		TxStatus committed = Committed;
		if (desc->status.compare_exchange_strong(active, committed)) {
			MarkDelete(delnodes, desc);
		}
	}
	else {
		TxStatus active = Active;
		TxStatus aborted = Aborted;
		desc->status.compare_exchange_strong(active, aborted);
	}
}

// Algorithm 6: Template for Transformed Insert Function
bool InsertNode(int key, void *val, Desc* desc, int opid) {
	NodeInfo* info = new NodeInfo; // TODO: Optionally preallocate these.
	info->desc = desc; info->opid = opid;
	Status ret = retry;
	while (true) {
		Node* curr = Do_LocatePred(key);
		if (IsNodePresent(curr, key)) {
			ret = UpdateNodeInfo(curr, info, true);
		}
		else {
			Node* n = new Node; // TODO: Optionally preallocate these.
			n->key = key; n->val = val; n->info = info;
			ret = (Do_Insert(n) ? success : fail);
		}
		if (ret == success) {
			return true;
		}
		else if (ret == fail) {
			return false;
		}
	}
}

// Algorithm 7: Template for Transformed Find Function
void *FindNode(int key, Desc* desc, int opid) {
	NodeInfo* info = new NodeInfo; // TODO: Optionally preallocate these.
	info->desc = desc; info->opid = opid;
	Status ret = retry;
	while (true) {
		Node* curr = Do_LocatePred(key);
		if (IsNodePresent(curr, key)) {
			ret = UpdateNodeInfo(curr, info, true);
		}
		else {
			ret = fail;
		}
		if (ret == success) {
			return curr->val;
		}
		else if (ret == fail) {
			return nullptr;
		}
	}
}

// Algorithm 8: Template for Transformed Delete Function
bool DeleteNode(int key, Desc* desc, int opid, Node*& del) {
	NodeInfo* info = new NodeInfo; // TODO: Optionally preallocate these.
	info->desc = desc; info->opid = opid;
	Status ret = retry;
	while (true) {
		Node* curr = Do_LocatePred(key);
		if (IsNodePresent(curr, key)) {
			ret = UpdateNodeInfo(curr, info, true);
		}
		else {
			ret = fail;
		}
		if (ret == success) {
			del = curr; // TODO: Make sure this works as intended. We want to pass the pointer back to the calling function.
			return true;
		}
		else if (ret == fail) {
			del = nullptr; // TODO: Make sure passing back nullptr doesn't have an adverse affect on our set.
			return false;
		}
	}
}
void MarkDelete(set<Node*> delnodes, Desc* desc) { // TODO: Completely change this behavior if we preallocate nodes.
	set<Node*>::iterator del = delnodes.begin();
	while (del != delnodes.end()) {
		// If a nullptr value was added to the set.
		if (*del == nullptr) {
			continue;
		}
		NodeInfo* info = (*del)->info.load();
		// If the node has already been removed.
		if (info->desc != desc) {
			continue;
		}
		NodeInfo* unmarked = info;
		NodeInfo* marked = (NodeInfo*)SET_MARK(info);
		if ((*del)->info.compare_exchange_strong(unmarked, marked)) {
			Do_Delete(*del);
		}
		del++;
	}
}
