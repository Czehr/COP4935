To build and run the assignment programs, simply run the following commands in a terminal with make and g++ installed:

To build the executables:
make

To run the program:
./LFTT.out


The number of threads, number of operations, operation distributions, transaction sizes, etc. can be manipulated through the source code, as they have been implemented using global constants.



Approach, efficiency, and correctness:

The nonblocking list is very similar to our blocking list, except that it does not use locks. Instead, it utilizes atomic instructions. The idea behind this implementation is to use an atomic markable reference. This reference stores both a pointer and an address in a single atomic value. Each node uses one of these types of pointers to point to the next node. If the reference bit in the node's pointer is true, then the node's successor is logically deleted. Otherwise it's logically inserted. We can linearize removals when the logical removal occurs and have all threads share the work of the physical deletion later if needed. We benefit from making each node's pointer atomic because we can linearize when a pointer change occurs. For instance, an insertion linearizes upon success when the new node's predecessor’s pointer changes to point to the new node.
The transactional component was added based on the LFTT paper, and maintains the correctness guarantees defined there. In short, if a thread's transaction is invalidated at any point, it will help complete the other transaction that blocked it before trying again. This ensures lock freedom, because some thread will always make progress. 
Lock-free transactions allow programs to produce much more efficiency than anything we have practiced implementing, and the complexity of this program we just implemented justifies the purpose of our project. It is very powerful, useful, and efficient, yet requires lots of programming effort to implement.
The correctness of this program is mostly dependent on our implementation skills, where we gathered most of our information through Zhang’s paper. As per our benchmarks, we believe that our program is correctly implemented and represents the amount of efficiency gains expected. Using C++11’s atomic operations library along with transaction status and descriptors required lots of attention to detail as one single mistake could disperse and cause bigger clashes. Following the provided path for us, we were able to simply guide our threads, and have them collaborate with the transactions using their descriptors, to yield as much successes as we could ideally get. Additionally, a failing transaction’s impact is easier to reduce using the transaction status, and does not necessarily cause wasting computation power. 
Since we relied mostly on a given idea on how to implement this, along with our implementation skills, and benchmarks, we can safely say that our efficiency represents the experimental evaluations that are in Zhang’s paper.