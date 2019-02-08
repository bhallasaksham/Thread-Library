#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "context.h"
#include "preempt.h"
#include "queue.h"
#include "uthread.h"

#define USHRT_MAX 65535

typedef struct {
	uthread_t TID;
	int retval;
	void* top_of_stack;
	uthread_ctx_t context;
}TCB;

typedef struct {
	TCB* tcb;
	int retval;
}EXITED_TCB;

typedef struct {
	TCB* tcb;
	uthread_t TID;
}BLOCKED_TCB;

uthread_t thread_count = 0;
queue_t global_queue;
queue_t zombie_queue;
queue_t blocked_queue;
TCB* active_tcb;

/* find item in global queue, callback function for queue_iterate */
static int find_item_in_global_queue(void *data, void *arg) {
    TCB *a = (TCB*)data;
    uthread_t *match = (uthread_t*)arg;
    if (a->TID == *match)
        return 1;

    return 0;
}

/* find item in zombie queue, callback function for queue_iterate */
static int find_item_in_zombie_queue(void *data, void *arg) {
	EXITED_TCB *a = (EXITED_TCB*)data;
	uthread_t *match = (uthread_t*)arg;	
	if (a->tcb->TID == *match)
		return 1;

	return 0;
}

/* find item in blocked queue, callback function for blocked_queue */
static int find_item_in_blocked_queue(void  *data, void *arg) {
	BLOCKED_TCB *a = (BLOCKED_TCB*)data;
	uthread_t *match = (uthread_t*)arg;
	if (a->TID == *match)
		return 1;

	return 0;
}

/* initialize a new thread */
TCB* uthread_init(uthread_t TID, void* stack, uthread_ctx_t context) {
	TCB* tcb = (TCB*)malloc(sizeof(TCB));
	tcb->TID = TID;
	tcb->top_of_stack = stack;
	tcb->context = context;
	return tcb;
}

/* creating the first thread, this is called when uthread_create is called for the first time */
int create_first_thread(uthread_func_t func, void *arg) {
	/* allocate space for main's context */
	uthread_ctx_t* main_context = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));
	if (main_context == NULL) {
		return -1;
	}
	/* allocate space for the new thread */
	uthread_ctx_t next_thread_context;

	/* main is the currently running thread, it is saved in a global variable active_tcb */ 
	active_tcb = uthread_init(thread_count, NULL, *main_context);
	if (active_tcb == NULL) {
		return -1;
	}
	/* thread_count incremented every time new thread is created */
	thread_count += 1;
	/* allocate stack for new thread */
	void* stack = uthread_ctx_alloc_stack();
	if (stack == NULL) {
		return -1;
	}
	/* initialize context for new thread */
	int ret = uthread_ctx_init(&next_thread_context, stack, func, arg);
	if (ret == -1) {
		return -1;
	}
	/* initialize a new thread */
	TCB* tcb = uthread_init(thread_count, stack, next_thread_context);
	if (tcb == NULL) {
		return -1;
	}
	/* thread_count incremented every time new thread is created */
	thread_count += 1;
	/* create a new queue, this queue has all threads which are ready to execute */
	global_queue = queue_create();
	if (global_queue == NULL) {
		return -1;
	}
	/* new thread created is ready for execution */
	queue_enqueue(global_queue, tcb);	
	/* return TID of new thread created */
	return tcb->TID;
}

int create_new_thread(uthread_func_t func, void *arg) {
	/* allocate stack for new thread */
	void* stack = uthread_ctx_alloc_stack();
	if (stack == NULL) {
		return -1;
	}
	/* initialize context for new thread */
	uthread_ctx_t next_thread_context;
	int ret = uthread_ctx_init(&next_thread_context, stack, func, arg);
	if (ret == -1) {
		return -1;
	}
	/* initialize a new thread */
	TCB* tcb = uthread_init(thread_count, stack, next_thread_context);
	if (tcb == NULL) {
		return -1;
	}
	/* thread_count incremented every time new thread is created */
	thread_count += 1;
	/* new thread created is ready for execution */
	queue_enqueue(global_queue, tcb);
	/* return TID of new thread created */
	return tcb->TID;
}

/* when a thread exits, it should check if there are processes in the blocked queue which are waiting for this thread to exit */
void check_blocked_threads(int retval) {
	/* BLOCKED_TCB struct contains the the tcb that is blocked and the tid because of which it is blocked */
	BLOCKED_TCB* blocked_tcb = NULL;
	/* check if the exiting thread's TID is in the blocked queue */
	uthread_t *ptr = &active_tcb->TID;
	queue_iterate(blocked_queue, find_item_in_blocked_queue, (void*)ptr, (void**)&blocked_tcb);
	/* if TID is found in blocked queue, then a thread was waiting for the currently active thread to exit */
	if (blocked_tcb != NULL) {
		/* the blocked thread is now ready for execution, it is enqueued in global_queue and deleted from blocked_queue */
		blocked_tcb->tcb->retval = retval;
		queue_enqueue(global_queue, blocked_tcb->tcb);
		queue_delete(blocked_queue, blocked_tcb);
		thread_count += 1;
	}
}

/* when a thread exits, it is enqueued into the zombie_queue, it will stay there until another thread collects its value */
void add_to_exit_queue(int retval) {
	/* EXITED_TCB struct contains the tcb that has exited and the return value that it exited with */
	EXITED_TCB* exited_tcb = (EXITED_TCB*)malloc(sizeof(EXITED_TCB));
	exited_tcb->tcb = active_tcb;
	exited_tcb->retval = retval;
	if (zombie_queue == NULL) {
		zombie_queue = queue_create();
	}
	queue_enqueue(zombie_queue, exited_tcb);
}

/* check if thread with TID = tid is already joined by another thread */
int check_in_blocked_queue(uthread_t tid) {
	BLOCKED_TCB* tcb = NULL;
	uthread_t *ptr = &tid;
	queue_iterate(blocked_queue, find_item_in_blocked_queue, (void*)ptr, (void**)&tcb);
	if (tcb != NULL) {
		return -1;
	}
	return 0;
}

/* check if thread with TID = tid is present in global queue or zombie queue */
int check_in_global_queue(uthread_t tid, int *retval) {
	TCB* found_tcb = NULL;
	uthread_t *ptr = &tid;
	queue_iterate(global_queue, find_item_in_global_queue, (void*)ptr, (void**)&found_tcb);
	/* if thread is not found in the global queue */
	if (found_tcb == NULL) {
		/* check for thread in exited queue */
		EXITED_TCB* exited_tcb = NULL;
		queue_iterate(zombie_queue, find_item_in_zombie_queue, (void*)ptr, (void**)&exited_tcb);
		/* if thread is not found in the exited  queue */
		if (exited_tcb == NULL) {
			return -1;
		}
		/* if thread has already exited, collect its return value */
		if (retval == NULL) {
			retval = &active_tcb->retval;
		} else {
			*retval = active_tcb->retval;
		}
		/* delete thread from zombie queue */
		queue_delete(zombie_queue, exited_tcb);
	}
	return 0;
}

/* when a thread is waiting for another thread to exit, it must be added to blocked queue */
void add_to_blocked_queue(uthread_t tid, int *retval) {
	preempt_disable();
	/* blocked tcb consists of the tcb of the blocked thread and the TID of the thread it is waiting to complete */
	BLOCKED_TCB* blocked_tcb = (BLOCKED_TCB*)malloc(sizeof(BLOCKED_TCB));
	blocked_tcb->tcb = active_tcb;
	blocked_tcb->TID = tid;
	if (blocked_queue == NULL) {
		blocked_queue = queue_create();
	}
	queue_enqueue(blocked_queue, blocked_tcb);
	/* dequeue an element from the ready threads to begin executing it */
	TCB* dequeued_element = NULL;
	TCB* temp = (TCB*)malloc(sizeof(TCB));
	temp = blocked_tcb->tcb;
	queue_dequeue(global_queue, (void**)&dequeued_element);
	active_tcb = dequeued_element;
	preempt_enable();
	uthread_ctx_switch(&temp->context, &dequeued_element->context);
	/* collect the return value of the thread */
	if (retval == NULL) {
		retval = &active_tcb->retval;
	} else {
		*retval = active_tcb->retval;
	}
}

/* API function that yields and gives control to the next thread. Implementing round robin method for yielding here */
void uthread_yield(void)
{	
	/* when changing global variables, disable preemption */
	preempt_disable();
	int size = queue_length(global_queue);
	/* if there are threads in the global queue waiting for execution */
	if (size > 0) {
		/* dequeue element from front of the queue */
		TCB* dequeued_element;
		TCB* temp = (TCB*)malloc(sizeof(TCB));
		queue_dequeue(global_queue, (void**)&dequeued_element);
		/* enqueue the currently running process at the end of the queue */
		queue_enqueue(global_queue, active_tcb);
		temp = active_tcb;
		active_tcb = dequeued_element;
		/* no more changing global variables, preemption can resume */
		preempt_enable();
		/* switch context between currently running process and the process that was just dequeued */
		uthread_ctx_switch(&temp->context, &dequeued_element->context);
	} 
}

/* API function that returns the TID of the currently running thread */
uthread_t uthread_self(void)
{
	return active_tcb->TID;
}

/* API function that creates a new thread */
int uthread_create(uthread_func_t func, void *arg)
{
	/* get the return value from the functions */
	int ret = 0;
	/* user cannot create more than USHRT_MAX threads */
	if (thread_count == USHRT_MAX) {
		return -1;
	}
	if (thread_count == 0) {
		/* Called to create the first thread */
		ret = create_first_thread(func, arg);	
		/* preemption starts as soon as the first thread is created */
		preempt_start();
		return ret;
	} else {
		/* disable preemption before changing global variables */
		preempt_disable();
		ret = create_new_thread(func, arg);
		/* enable preemption once global variables are changed */
		preempt_enable();
		return ret;
	}
	return 0;
}


/* API function that handles exiting of the currently running thread */
void uthread_exit(int retval)
{
	/* disable preemption before changing global variables */
	preempt_disable();
	/* check if there is a process in blocked_queue waiting for this process to exit */
	check_blocked_threads(retval);
	/* once a process exits, it is pushed into the exit queue where it waits to be collected by the parent */
	add_to_exit_queue(retval);

	/* check if more jobs are present in global_queue waiting to get executed */
	int size = queue_length(global_queue);
	if (size != 0) {
		/* if a thread exits, a new thread is dequeued from the queue of ready threads and it becomes the active process */
		TCB* dequeued_element = NULL;
		TCB* temp = (TCB*)malloc(sizeof(TCB));
		temp = active_tcb;
		queue_dequeue(global_queue, (void**)&dequeued_element);
		active_tcb = dequeued_element;
		preempt_enable();
		uthread_ctx_switch(&temp->context, &dequeued_element->context);
	}
	/* enable preemption once global variables are changed */
	preempt_enable();
}

int uthread_join(uthread_t tid, int *retval)
{	
	/* thread should not be allowed to join the parent */
	if (tid == 0) {
		return -1;
	}
	/* thread should not be allowed to join itself */
	if (tid == active_tcb->TID) {
		return -1;
	}
	/* check if thread is already joined */
	int ret = check_in_blocked_queue(tid);
	if (ret == -1) {
		return -1;
	}

	/* check if thread to join is in global_queue or zombie_queue */
	ret = check_in_global_queue(tid,retval);
	if (ret == -1) {
		return -1;
	}
	/* add the current process into blocked_queue */
	add_to_blocked_queue(tid, retval);
	return 0;
}

