# Project 2 - User-Level Thread Library
## Project Partners
Saksham Bhalla and Jhilmil Malhotra

## INTRODUCTION
This project helped us understand how threads work, and also how to write  
tests for our code. One of the great challenges in this project was to conform  
to the API standards. It was also helpful to learn how to write tests after  
API implementation.  

## PHASE 1 - Implementation of the queue API
To implement a generic API for the queue, it must be able to support  
any data type. In order to make most of the operations `O(1)`, we decided  
to implement our queue as a linked list. Each node in the linked list  
contains a `void* data` and a pointer to the next node. The queue data  
structure consists of front and rear pointers for the queue and a  
Node object for each node. Once we chose the right data structures,  
the implementation was very straightforward.  

### Testing the queue API

To test whether our API is truly generic, we decided to perform all queue  
operations on 2 different data types. To make sure our queue handles  
all the edge cases, we wrote a total of 29 tests.  

## PHASE 2 - Implementation of the uthread API
Implementing the uthread library was the most challenging part of this  
assignment. Since it was a completely new process, it took us a long time  
to understand how threads work.  

In order to implement the first function `uthread_create()`, we needed to  
understand the context API and how to handle the main thread differently.  
Since main thread was the currently running thread, we understood that we do  
not need to allocate stack or initialize context for this thread. However, it  
would be helpful to define a context object for main in order to save its  
context when it yields to another thread. Once we understood that, we were  
able to implement the`uthread_create()` method easily.

The next step was to implement `uthread_yield()` function. To implement this,  
we learned the round-robin method which says that the currently running thread  
must yield control to the next available thread in the queue and should be  
enqueued. Our implementation was such that the currently running thread is  
never present in the queue and the queue only has threads which are waiting  
to be executed.  

The next step was to implement `uthread_exit()` function which simply   
terminates the currently active thread and yields control to the next available  
thread in the queue. The only difference between yield at exit at this point is  
that the exit function doesn't enqueue the currently active thread back in the  
queue. 

Finally, implementing `uthread_join()` function was very straightforward.  
We had an infinite loop which terminates when there are no more jobs in the  
queue. Otherwise we just simply yield to the next thread in the queue. 

### Testing the uthread API
In order to test the uthread API, we used the 2 test cases provided to us  
and we were able to get the expected output.  

## PHASE 3 - Implementation of `uthread_join()`
Once we understood how threads work, implementing `uthread_join()` was  
relatively easier. We learned that using one queue for everything might make  
the job hard therefore we decided to use three different queues. We maintained  
a ready queue for all ready threads, a zombie queue for all exited threads and   
a blocked queue for all blocked threads. The zombie queue stores the exited   
thread's tcb and the return value whereas the blocked queue stores the blocked  
thread's tcb as well as the TID of the thread because of which it is blocked.  
Once we understood that, we just had to follow the instructions on the  
assignment.  

### Testing `uthread_join()` 
In order to test joining, we wrote a new test script called `uthread_join`.  
This testing script tests whether the parent threads properly wait for the   
child threads. It also checks if the return value is properly collected from   
the child.  

## PHASE 4 - Implementation of preemption
To implement preemption, we had to configure a signal handler and a timer.  
Implementing those required us to carefully go over the GNU documentation  
but it was not very hard to learn. Enabling and disabling preemption required  
us to use sigprocmask and the provided documentation on that was very helpful.  

### Testing preemption
WARNING: `test_preempt.c` has an infinite loop.  
In order to test preemption, we learned that the threads need to run for a  
long time and we need to check if the threads yield to the next available  
thread in the queue. In order to test that, we wrote a test that creates  
3 different threads and all threads execute indefinitely. We see that the  
threads automatically yield to the next available thread which confirms  
that preemption works.  

## CITATIONS 

https://www.geeksforgeeks.org/queue-set-2-linked-list-implementation/  
https://pseudomuto.com/2013/05/implementing-a-generic-linked-list-in-c/  
https://www.geeksforgeeks.org/generic-linked-list-in-c-2/  
https://www.geeksforgeeks.org/linked-list-set-3-deleting-node/  
http://www.informit.com/articles/article.aspx?p=23618&seqNum=14  
https://www.gnu.org/software/libc/manual/html_mono/libc.html  
