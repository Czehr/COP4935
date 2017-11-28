#pragma once

#include <atomic>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include "SkipListNode.h"
#include "LogicalStatus.h"

void init();
Node* Do_LocatePred(int key);
bool Do_Insert(Node* n);
bool Do_Delete(Node* n);
bool Do_Find(int key);