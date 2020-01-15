#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <uthread.h>

int thread5(void* arg)
{
	printf("thread%d\n", uthread_self());
	return 0;
}

int thread4(void* arg)
{
	uthread_join(uthread_create(thread5, NULL), NULL);
	int ret = uthread_join(1, NULL);
	assert(ret == -1);
	printf("thread%d\n", uthread_self());
	return 0;
}

int thread3(void* arg)
{
	uthread_join(uthread_create(thread4, NULL), NULL);
	int ret = uthread_join(1, NULL);
	assert(ret == -1);
	printf("thread%d\n", uthread_self());
	return 0;
}

int thread2(void* arg) {
	uthread_join(uthread_create(thread3, NULL), NULL);
	int num1 = 4;
	int num2 = 5;
	int ret = num1 + num2;
	printf("thread%d\n", uthread_self());
	return ret;
}

int thread1(void* arg) {
	int ret = 0;
	uthread_join(uthread_create(thread2, NULL), &ret);
	ret = ret*2;
	printf("thread%d\n", uthread_self());
	return ret;
}

int main(void)
{
	int ret = 0;
	uthread_join(uthread_create(thread1, NULL), &ret);
	assert(ret == 18);
	return 0;
}