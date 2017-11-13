#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstdlib>
#include <cstddef>
#include "LFTT.h"
#include "LFTTTypeDef.h"

// The following are adjustable to run the program with different sets of functionality.
const int THREAD_COUNT = 4;

const int PERCENT_INSERT = 33;
const int PERCENT_DELETE = 33;
// Because of the way our random function works, PERCENT_FIND does nothing.
// Set it by leaving a percentage left over from insert and delete.
const int PERCENT_FIND = 33;

const int KEY_RANGE = 10000;
const int NUM_TRANSACTIONS = 160000;
const bool DEBUG = false;
