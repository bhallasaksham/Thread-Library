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

To test whether our API is truly generic, we decided perform all queue 
operations on 2 different data types. To make sure our queue handles 
all the edge cases, we wrote a total of 29 tests.

## PHASE 2


## PHASE 3


## PHASE 4


## CITATIONS 

https://www.geeksforgeeks.org/queue-set-2-linked-list-implementation/ 
https://pseudomuto.com/2013/05/implementing-a-generic-linked-list-in-c/
https://www.geeksforgeeks.org/generic-linked-list-in-c-2/
https://www.geeksforgeeks.org/linked-list-set-3-deleting-node/
http://www.informit.com/articles/article.aspx?p=23618&seqNum=14