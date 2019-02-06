#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <uthread.h>

int thread2(void* arg) {
	int num1 = 4;
	int num2 = 5;
	int ret = num1 + num2;
	return ret;
}

int thread1(void* arg) {
	int ret = 0;
	uthread_join(uthread_create(thread2, NULL), &ret);
	ret = ret*2;
	return ret;
}

int main(void)
{
	int ret = 0;
	uthread_join(uthread_create(thread1, NULL), &ret);
	assert(ret == 18);
	return 0;
}