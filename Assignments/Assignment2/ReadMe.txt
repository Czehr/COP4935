To build and run the assignment programs, simply run the following commands in a terminal with make and g++ installed:

To build the executables:
make

To run the blocking set:
./BLOCKING_LIST.out

To run the non-blocking set:
./NONBLOCKING_LIST.out


The number of threads and number of operations can be manipulated through the source code
right where they have been implemented as global variables.
Also, the probability calculations have been commented out to be easily to implemented if needed.



Approach, efficiency, and correctness:

For the blocking list, we used a single lock to maintain correctness. All threads that perform one of the set's public operations must first obtain this lock. Thus, our blocking implementation is essentially a sequential set implementation. This lacks efficiency, but guarantees correctness. To be more specific, the set uses a linked-list structure, with sentinel nodes holding the maximum and minimum values. Nodes are ordered by key. 

The nonblocking list is very similar to our blocking list, except that it does not use locks. Instead, it utilizes atomic instructions. The idea behind this implementation is to use an atomic markable reference. This reference stores both a pointer and an address in a single atomic value. Each node uses one of these types of pointers to point to the next node. If the reference bit in the node's pointer is true, then the node's successor is logically deleted. Otherwise it's logically inserted. We can linearize removals when the logical removal occurs and have all threads share the work of the physical deletion later if needed. We benefit from making each node's pointer atomic because we can linearize when a pointer change occurs. For instance, an insertion linearizes upon success when the new node's predecessor’s pointer changes to point to the new node.