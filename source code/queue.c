#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "queue.h"

struct Node { 
	void *data;
	struct Node* next;
};

struct queue {
	int sizeofQueue;
	struct Node *front, *rear;
};

queue_t queue_create(void)
{
	queue_t my_queue;
	/* will return NULL in case of failure, else returns the address of newly allocated memory */
	my_queue = (queue_t)malloc(sizeof(queue_t));

	if (my_queue != NULL) {
		/* initialize data in queue */
		my_queue->front = my_queue->rear = NULL;
		my_queue->sizeofQueue = 0;		
	}

	/* returning pointer to the empty queue, NULL in case of failure */
	return my_queue; 
}

int queue_destroy(queue_t queue)
{	
	/* making sure implementation doesnt crash on recieving empty arguments */
	if (queue == NULL) {
		return -1;
	}

	if (queue->front != NULL) {
		return -1;
	}
	/* Deallocating the memory associated to the queue object pointed by queue. */
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	/* if queue doesn't exist or no data passed in this function */
	if (queue == NULL || data == NULL) {
		return -1;
	}
	/* Allocating adequate memory to the node */
	struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
	/* if memory not allocated correctly */
	if (new_node == NULL) {
		return -1;
	}
	new_node->data = data;
	/* Since insertion always happens at the end */
	new_node->next = NULL;
	/* If no data is present in the queue. */
	if (queue->rear == NULL) {
		queue->front = queue->rear = new_node;	
	}
	/* If there is already data in the queue. */
	else {
		queue->rear->next = new_node;
		queue->rear = new_node;
	}
	queue->sizeofQueue += 1;
	/* Successfully enqueued */
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	/* if queue doesn't exist or no data passed in this function */
	if (queue == NULL || data == NULL) {
		return -1;
	}
	/* If no data is present in the queue. */
	if (queue->front == NULL) {
		return -1;
	}
	/* Declare a temp instance of the node to extract data from node that is to be dequeued */
	struct Node* temp_node = (struct Node*)malloc(sizeof(struct Node));
	/* Copy the first node in the queue to temp_node */
	memcpy(temp_node, queue->front, sizeof(struct Node));
	*data = temp_node->data;
	/* queue_front is the 2nd element in the queue, which means first element is dequeued */
	queue->front = queue->front->next;
	/* delete temp_node */
	free(temp_node);
	/* If the dequeued element was the last element */
	if (queue->front == NULL) {
		queue->rear = NULL;
	}

	queue->sizeofQueue -= 1;
	/* Successfully dequeued */
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	/* if queue doesn't exist or no data passed in this function */
	if (queue == NULL || data == NULL) {
		return -1;
	}
	/* If no data is present in the queue. */
	if (queue->front == NULL) {
		return -1;
	}

	/* Node to delete */
	struct Node* temp = queue->front;
	/* In order to keep track of the previous node */
	struct Node* prev = temp;

	/* If the data we're looking for is in the front of the queue */
	if (temp != NULL && data == temp->data) {
		queue->front = temp->next;
		queue->sizeofQueue -= 1;
		free(temp);
		return 0;
	}

	/* Searching for data in the entire queue */
	while (temp != NULL) {
		/* If found */
		if (data == temp->data) {
			break;
		} 
		/* Keep track of previous node */
		prev = temp;
		/* Go to next node */
		temp = temp->next;
	}

	/* If element not found */
	if (temp == NULL) {
		return -1;
	}

	/* The next of prev is now the next of temp, meaning we skip the node temp */
	prev->next = temp->next;
	/* Element deleted from the queue */
	queue->sizeofQueue -= 1;
	free(temp);
	/* Successfully deleted */
	return 0;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
	/* if queue doesnt exist or function pointer not set */
	if (queue == NULL || func == NULL) {
		return -1;
	}

	/* iterate through the queue and call the function on each element in the queue */
	for (struct Node* temp = queue->front; temp != NULL; temp = temp->next) {
		int ret = func(temp->data, arg);
		/* if function returns 1 and data needs to be set */
		if (ret == 1 && data != NULL) {
			*data = temp->data;
			break;
		}
	}
	/* Successfully iterated */
	return 0;
}

int queue_length(queue_t queue)
{	
	/* returns the size of queue if queue exists, NULL otherwise */
	if (queue != NULL) {
		return queue->sizeofQueue;
	} else {
		return -1;
	}
	
}

