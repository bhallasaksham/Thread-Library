#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <preempt.h>
#include <uthread.h>

int thread3(void* arg)
{
	while (1) {
		printf("thread%d\n", uthread_self());
	}
	return 0;
}

int thread2(void* arg)
{
	uthread_create(thread3, NULL);
	while (1) {
		printf("thread%d\n", uthread_self());
	}
	return 0;
}

int thread1(void* arg)
{
	uthread_create(thread2, NULL);
	while (1) {
		printf("thread%d\n", uthread_self());
	}
	return 0;
}

int main(void) {
	uthread_join(uthread_create(thread1, NULL), NULL);
}