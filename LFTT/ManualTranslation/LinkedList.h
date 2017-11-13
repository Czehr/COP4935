#pragma once

#include <atomic>
#include <limits.h>
#include "LinkedListNode.h"
#include "LFTT.h"
#include "LogicalStatus.h"

void init();
Node* Do_LocatePred(int key);
bool Do_Insert(Node* n);
bool Do_Delete(Node* n);
bool Do_Find(int key);