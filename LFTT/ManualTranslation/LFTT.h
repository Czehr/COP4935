#ifndef LFTT_H
#define LFTT_H

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <set>
#include <assert.h>
#include "LFTTTypeDef.h"
#include "LogicalStatus.h"
#include "LinkedList.h" // Change this to reference the data structure you want to use.

// This operation is included in the header so that other files can call it.
bool ExecuteTransaction(Desc* desc);

#endif LFTT_H