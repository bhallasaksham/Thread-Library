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

/* TODO Phase 2 */

struct TCB {
	uthread_t TID;
	int state;
	void* top_of_stack;
	uthread_ctx_t context;
};

uthread_t thread_count = 0;
queue_t global_queue;
struct TCB* active_tcb;

struct TCB* uthread_init(uthread_t TID, int state, void* stack, uthread_ctx_t context) {
	struct TCB* tcb = (struct TCB*)malloc(sizeof(struct TCB));
	tcb->TID = TID;
	tcb->state = state;
	tcb->top_of_stack = stack;
	tcb->context = context;
	return tcb;
}


void uthread_yield(void)
{	
	int size = queue_length(global_queue);
	if (size > 0) {
		struct TCB* dequeued_element;
		struct TCB* temp = (struct TCB*)malloc(sizeof(struct TCB));
		queue_dequeue(global_queue, (void**)&dequeued_element);
		active_tcb->state = 1;
		queue_enqueue(global_queue, active_tcb);
		temp = active_tcb;
		active_tcb = dequeued_element;
		active_tcb->state = 0;
		// printf("ACTIVE TCB: %d\n", temp->TID);
		// printf("CONTEXT SWITCH WITH TCB: %d\n", dequeued_element->TID);
		uthread_ctx_switch(&temp->context, &dequeued_element->context);
	}
	
}

uthread_t uthread_self(void)
{
	return active_tcb->TID;
}

int uthread_create(uthread_func_t func, void *arg)
{
	if (thread_count == 0) {
		uthread_ctx_t* main_context = (uthread_ctx_t*)malloc(sizeof(uthread_ctx_t));
		uthread_ctx_t next_thread_context;
		active_tcb = uthread_init(thread_count, 0, NULL, *main_context);
		void* stack = uthread_ctx_alloc_stack();
		uthread_ctx_init(&next_thread_context, stack, func, arg);
		struct TCB* tcb = uthread_init(thread_count + 1, 1, stack, next_thread_context);
		thread_count = thread_count + 1;
		global_queue = queue_create();
		queue_enqueue(global_queue, tcb);
		return tcb->TID;
	} else {
		void* stack = uthread_ctx_alloc_stack();
		uthread_ctx_t next_thread_context;
		uthread_ctx_init(&next_thread_context, stack, func, arg);
		struct TCB* tcb = uthread_init(thread_count + 1, 1, stack, next_thread_context);
		thread_count = thread_count + 1;
		queue_enqueue(global_queue, tcb);
		return tcb->TID;
	}

	return 0;
}

void uthread_exit(int retval)
{
	if (retval == 0) {
		int size = queue_length(global_queue);
		if (size != 0) {
			struct TCB* dequeued_element = NULL;
			struct TCB* temp = (struct TCB*)malloc(sizeof(struct TCB));
			temp = active_tcb;
			queue_dequeue(global_queue, (void**)&dequeued_element);
			active_tcb->state = 2;
			active_tcb = dequeued_element;
			active_tcb->state = 0;
			uthread_ctx_switch(&temp->context, &dequeued_element->context);
		}
	}
}

int uthread_join(uthread_t tid, int *retval)
{
	while(1) {
		int size = queue_length(global_queue);
		// printf("%d\n", size);
		if (size == 0) {
			break;
		}
		else {
			uthread_yield();
		}
	}
	/* TODO Phase 3 */
	return 0;
}

