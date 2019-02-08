#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <queue.h>
#include <queue.c>

typedef struct{
	int id;
	char* name;
}Student;

/* creates a new instance of student by initializing id and name to the function parameters */
Student* create_new_student(int id, char* name) {
	Student* student = (Student*)malloc(sizeof(Student));
	student->id = id;
	student->name = name;
	return student;
}

/* Callback function that increments items by a certain value, as given by instructor */
static int inc_item(void *data, void *arg)
{
    int *a = (int*)data;
    int inc = (int)(long)arg;

    *a += inc;

    return 0;
}

/* Callback function that finds a certain item according to its value, as given by instructor */
static int find_item(void *data, void *arg)
{
    int *a = (int*)data;
    int match = (int)(long)arg;

    if (*a == match)
        return 1;

    return 0;
}

/* Callback function for struct Student */
static int find_student(void *data, void *arg) {
	Student* a = (Student*)data;
	int match = (int)(long)arg;

	if (a->id == match)
		return 1;

	return 0;
}

void testForInts(queue_t my_queue) {
	int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	/* Test if enqueue works properly */
	for (int i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
		queue_enqueue(my_queue, &data[i]);	
	}

	/* Queue is not empty, destroying queue should not be valid */
	assert(queue_destroy(my_queue) == -1);

	/* Check if 10 items were enqueued */
	int size = queue_length(my_queue);
	assert(size == 10);
    /* Add value '1' to every item of the queue */
    queue_iterate(my_queue, inc_item, (void*)1, NULL);
    assert(data[0] == 2);
    /* Check if ptr is set properly in iterate function */
    int *ptr = NULL;
    queue_iterate(my_queue, find_item, (void*)5, (void**)&ptr);
    assert(ptr != NULL);
    assert(*ptr == 5);
    assert(ptr == &data[3]);
	/* Check if delete works, size of queue should be reduced to 9 */
	queue_delete(my_queue,&data[2]);
	size = queue_length(my_queue);
	assert(size == 9);
	/* Check if ptr is being set correctly in dequeue */
	queue_dequeue(my_queue, (void**)&ptr);
	assert(*ptr == 2);
	/* Check if all items can be dequeued and the size becomes zero at the end */
	for (int i = 0; i < size; i++) {
		queue_dequeue(my_queue, (void**)&ptr);
	}
	size = queue_length(my_queue);
	assert(size == 0);

	/* Check if queue successfully destroyed */
	int ret = queue_destroy(my_queue);
	assert(ret == 0);
}

void testForStructs(queue_t my_queue) {
	/* Create 3 new students */
	Student* student1 = create_new_student(1, "John");
	Student* student2 = create_new_student(2, "Sam");
	Student* student3 = create_new_student(3, "Michael");
	
	/* Test if enqueue works properly */
	queue_enqueue(my_queue, student1);
	queue_enqueue(my_queue, student2);
	queue_enqueue(my_queue, student3);


	/* Queue is not empty, destroying queue should not be valid */
	assert(queue_destroy(my_queue) == -1);

	/* Check if 3 items were enqueued */
	int size = queue_length(my_queue);
	assert(size == 3);

	/* Check if ptr is set properly in iterate function */
	Student *ptr = NULL;
    queue_iterate(my_queue, find_student, (void*)1, (void**)&ptr);
    assert(ptr != NULL);
    assert(strcmp(ptr->name, "John") == 0);

    /* Check if delete works, size of queue should be reduced to 2 */
	queue_delete(my_queue, student2);
	size = queue_length(my_queue);
	assert(size == 2);

	/* Check if ptr is being set correctly in dequeue */
	queue_dequeue(my_queue, (void**)&ptr);
    assert(strcmp(ptr->name, "John") == 0);
	size = queue_length(my_queue);
	assert(size == 1);

	/* Check if size becomes zero at the end */
	queue_dequeue(my_queue, (void**)&ptr);
    assert(strcmp(ptr->name, "Michael") == 0);
	size = queue_length(my_queue);
	assert(size == 0);

	/* Check if queue successfully destroyed */
	int ret = queue_destroy(my_queue);
	assert(ret == 0);
}

void testForEdgeCases(queue_t my_queue) {
	/* Passing null should return -1 */
	assert(queue_destroy(NULL) == -1);
	assert(queue_enqueue(NULL, NULL) == -1);
	assert(queue_dequeue(NULL, NULL) == -1);
	assert(queue_delete(NULL, NULL) == -1);
	assert(queue_iterate(NULL,NULL,NULL,NULL) == -1);
	assert(queue_length(NULL) == -1);

	/* Dequeuing/Deleting from the empty queue should not be possible */
	assert(queue_dequeue(my_queue, NULL) == -1);
	assert(queue_delete(my_queue, NULL) == -1);

	/* Check if queue successfully destroyed */
	int ret = queue_destroy(my_queue);
	assert(ret == 0);
}

int main(void) {
	queue_t my_queue;
	my_queue = queue_create();
	testForEdgeCases(my_queue);
	my_queue = queue_create();
	testForInts(my_queue);
	my_queue = queue_create();
	testForStructs(my_queue);
	printf("All test cases passed!\n");
}