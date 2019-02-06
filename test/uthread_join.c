#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <uthread.h>

int thread3(void* arg)
{
	printf("thread%d\n", uthread_self());
	return 0;
}

int thread2(void* arg)
{
	uthread_join(uthread_create(thread3, NULL), NULL);
	int ret = uthread_join(1, NULL);
	assert(ret == -1);
	printf("thread%d\n", uthread_self());
	return 0;
}

int thread1(void* arg)
{
	uthread_join(uthread_create(thread2, NULL), NULL);
	int ret = uthread_join(1, NULL);
	assert(ret == -1);
	printf("thread%d\n", uthread_self());
	return 0;
}

int main(void)
{
	uthread_join(uthread_create(thread1, NULL), NULL);
	int ret = uthread_join(0, NULL);
	assert(ret == -1);
	return 0;
}