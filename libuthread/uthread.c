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
	int state;
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

/* Callback function that finds a certain item according to its value */
static int find_item_in_global_queue(void *data, void *arg) {
    TCB *a = (TCB*)data;
    uthread_t *match = (uthread_t*)arg;

    if (a->TID == *match)
        return 1;

    return 0;
}

static int find_item_in_zombie_queue(void *data, void *arg) {
	EXITED_TCB *a = (EXITED_TCB*)data;
	uthread_t *match = (uthread_t*)arg;

	if (a->tcb->TID == *match)
		return 1;

	return 0;
}

static int find_item_in_blocked_queue(void  *data, void *arg) {
	BLOCKED_TCB *a = (BLOCKED_TCB*)data;
	uthread_t *match = (uthread_t*)arg;

	if (a->TID == *match)
		return 1;

	return 0;
}

TCB* uthread_init(uthread_t TID, int state, void* stack, uthread_ctx_t context) {
	TCB* tcb = (TCB*)malloc(sizeof(TCB));
	tcb->TID = TID;
	tcb->state = state;
	tcb->top_of_stack = stack;
	tcb->context = context;
	return tcb;
}


void uthread_yield(void)
{	
	printf("in yield..\n");
	preempt_disable();
	int size = queue_length(global_queue);
	if (size > 0) {
		TCB* dequeued_element;
		TCB* temp = (TCB*)malloc(sizeof(TCB));
		queue_dequeue(global_queue, (void**)&dequeued_element);
		active_tcb->state = 1;
		queue_enqueue(global_queue, active_tcb);
		temp = active_tcb;
		active_tcb = dequeued_element;
		active_tcb->state = 0;
		preempt_enable();
		uthread_ctx_switch(&temp->context, &dequeued_element->context);
	} 
}

uthread_t uthread_self(void)
{
	return active_tcb->TID;
}

int uthread_create(uthread_func_t func, void *arg)
{
	if (thread_count == USHRT_MAX) {
		return -1;
	}
	if (thread_count == 0) {
		uthread_ctx_t* main_context = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));
		if (main_context == NULL) {
			return -1;
		}
		uthread_ctx_t next_thread_context;
		active_tcb = uthread_init(thread_count, 0, NULL, *main_context);
		if (active_tcb == NULL) {
			return -1;
		}
		void* stack = uthread_ctx_alloc_stack();
		if (stack == NULL) {
			return -1;
		}
		int ret = uthread_ctx_init(&next_thread_context, stack, func, arg);
		if (ret == -1) {
			return -1;
		}
		TCB* tcb = uthread_init(thread_count + 1, 1, stack, next_thread_context);
		if (tcb == NULL) {
			return -1;
		}
		thread_count = thread_count + 1;
		global_queue = queue_create();
		if (global_queue == NULL) {
			return -1;
		}
		queue_enqueue(global_queue, tcb);		
		preempt_start();
		return tcb->TID;
	} else {
		preempt_disable();
		void* stack = uthread_ctx_alloc_stack();
		if (stack == NULL) {
			return -1;
		}
		uthread_ctx_t next_thread_context;
		int ret = uthread_ctx_init(&next_thread_context, stack, func, arg);
		if (ret == -1) {
			return -1;
		}
		TCB* tcb = uthread_init(thread_count + 1, 1, stack, next_thread_context);
		if (tcb == NULL) {
			return -1;
		}
		thread_count = thread_count + 1;
		queue_enqueue(global_queue, tcb);
		preempt_enable();
		return tcb->TID;
	}
	return 0;
}

void uthread_exit(int retval)
{
	preempt_disable();
	/* check if there is a process in blocked_queue waiting for this process to exit */
	BLOCKED_TCB* blocked_tcb = NULL;
	uthread_t *ptr = &active_tcb->TID;
	queue_iterate(blocked_queue, find_item_in_blocked_queue, (void*)ptr, (void**)&blocked_tcb);
	if (blocked_tcb != NULL) {
		blocked_tcb->tcb->retval = retval;
		queue_enqueue(global_queue, blocked_tcb->tcb);
		queue_delete(blocked_queue, blocked_tcb);
		thread_count = thread_count + 1;
	}
	active_tcb->state = 2;
	EXITED_TCB* exited_tcb = (EXITED_TCB*)malloc(sizeof(EXITED_TCB));
	exited_tcb->tcb = active_tcb;
	exited_tcb->retval = retval;
	if (zombie_queue == NULL) {
		zombie_queue = queue_create();
	}
	queue_enqueue(zombie_queue, exited_tcb);
	int size = queue_length(global_queue);
	if (size != 0) {
		TCB* dequeued_element = NULL;
		TCB* temp = (TCB*)malloc(sizeof(TCB));
		temp = active_tcb;
		queue_dequeue(global_queue, (void**)&dequeued_element);
		active_tcb = dequeued_element;
		active_tcb->state = 0;
		preempt_enable();
		uthread_ctx_switch(&temp->context, &dequeued_element->context);
	}
	preempt_enable();
}

int uthread_join(uthread_t tid, int *retval)
{
	preempt_disable();
	if (tid == 0) {
		preempt_enable();
		return -1;
	}
	if (tid == active_tcb->TID) {
		preempt_enable();
		return -1;
	}
	BLOCKED_TCB* tcb = NULL;
	uthread_t *ptr = &tid;
	queue_iterate(blocked_queue, find_item_in_blocked_queue, (void*)ptr, (void**)&tcb);
	if (tcb != NULL) {
		preempt_enable();
		return -1;
	}
	TCB* found_tcb = NULL;
	EXITED_TCB* exited_tcb = NULL;
	queue_iterate(global_queue, find_item_in_global_queue, (void*)ptr, (void**)&found_tcb);
	if (found_tcb == NULL) {
		queue_iterate(zombie_queue, find_item_in_zombie_queue, (void*)ptr, (void**)&exited_tcb);
		if (exited_tcb == NULL) {
			preempt_enable();
			return -1;
		}
		if (retval == NULL) {
			retval = &active_tcb->retval;
		} else {
			*retval = active_tcb->retval;
		}
		queue_delete(zombie_queue, exited_tcb);
		preempt_enable();
		return 0;
	}
	BLOCKED_TCB* blocked_tcb = (BLOCKED_TCB*)malloc(sizeof(BLOCKED_TCB));
	active_tcb->state = 3;
	blocked_tcb->tcb = active_tcb;
	blocked_tcb->TID = tid;
	if (blocked_queue == NULL) {
		blocked_queue = queue_create();
	}
	queue_enqueue(blocked_queue, blocked_tcb);
	TCB* dequeued_element = NULL;
	TCB* temp = (TCB*)malloc(sizeof(TCB));
	temp = blocked_tcb->tcb;
	queue_dequeue(global_queue, (void**)&dequeued_element);
	active_tcb = dequeued_element;
	active_tcb->state = 0;
	preempt_enable();
	uthread_ctx_switch(&temp->context, &dequeued_element->context);
	if (retval == NULL) {
		retval = &active_tcb->retval;
	} else {
		*retval = active_tcb->retval;
	}

	return 0;
}

