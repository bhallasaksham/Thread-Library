#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <queue.h>
#include <queue.c>


/* Callback function that increments items by a certain value */
static int inc_item(void *data, void *arg)
{
    int *a = (int*)data;
    int inc = (int)(long)arg;

    *a += inc;

    return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(void *data, void *arg)
{
    int *a = (int*)data;
    int match = (int)(long)arg;

    if (*a == match)
        return 1;

    return 0;
}


void testForInts(queue_t my_queue) {
	char data[] = {'a', 'e', 'i', 'o', 'u'};
	/* TEST ENQUEUE */
	for (int i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
		queue_enqueue(my_queue, &data[i]);	
	}

    /* Add value '1' to every item of the queue */
    queue_iterate(my_queue, inc_item, (void*)1, NULL);
    assert(data[0] == 'b');
    int *ptr = NULL;
    queue_iterate(my_queue, find_item, (void*)'f', (void**)&ptr);

	/* TEST_DELETE */
	queue_delete(my_queue,&data[0]);

	/* TEST DEQUEUE */
	int size = queue_length(my_queue);
	for (int i = 0; i < size; i++) {
		queue_dequeue(my_queue, (void**)&ptr);
	}
	size = queue_length(my_queue);
	assert(size == 0);



}

int main(void) {
	queue_t my_queue;
	my_queue = queue_create();
	assert(queue_destroy(NULL) == -1);
	testForInts(my_queue);
	// printQueue(my_queue);
}