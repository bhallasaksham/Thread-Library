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
	int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	/* TEST ENQUEUE */
	for (int i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
		queue_enqueue(my_queue, &data[i]);	
	}

    /* Add value '1' to every item of the queue */
    queue_iterate(my_queue, inc_item, (void*)1, NULL);
    assert(data[0] == 2);
    int *ptr = NULL;
    queue_iterate(my_queue, find_item, (void*)5, (void**)&ptr);
    assert(ptr != NULL);
    assert(*ptr == 5);
    assert(ptr == &data[3]);

	/* TEST_DELETE */
	queue_delete(my_queue,&data[0]);

	/* TEST DEQUEUE */
	printf("Dequeued elements - \n");
	int size = queue_length(my_queue);
	printf("Size - %d\n", size);
	printf("[");
	for (int i = 0; i < size; i++) {
		queue_dequeue(my_queue, (void**)&ptr);
		if (i == (size- 1)) {
			printf("%d", *ptr);
		}
		else {
			printf("%d, ", *ptr);	
		}
	}
	printf("]\n");




}

int main(void) {
	queue_t my_queue;
	my_queue = queue_create();
	// assert(queue_destroy(NULL) == -1);
	testForInts(my_queue);
	// printQueue(my_queue);
}