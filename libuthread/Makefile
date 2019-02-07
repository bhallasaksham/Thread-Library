all : libuthread.a

libuthread.a : queue.o uthread.o context.o preempt.o
	ar rcs libuthread.a queue.o uthread.o context.o preempt.o

queue.o : queue.c
	gcc -g -Wall -Werror -c queue.c

context.o : context.c
	gcc -g -Wall -Werror -c context.c

uthread.o: uthread.c queue.c context.c
	gcc -g -Wall -Werror -c uthread.c
	gcc -g -Wall -Werror -c queue.c
	gcc -g -Wall -Werror -c context.c

preempt.o: preempt.c uthread.c
	gcc -g -Wall -Werror -c preempt.c
	gcc -g -Wall -Werror -c uthread.c

clean : 
	rm -rf libuthread.a *.o 
