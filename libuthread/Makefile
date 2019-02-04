all : libuthread.a

libuthread.a : queue.o uthread.o context.o
	ar rcs libuthread.a queue.o uthread.o context.o

queue.o : queue.c
	gcc -g -Wall -Werror -c queue.c

context.o : context.c
	gcc -g -Wall -Werror -c context.c

uthread.o: uthread.c queue.c context.c
	gcc -g -Wall -Werror -c uthread.c
	gcc -g -Wall -Werror -c queue.c
	gcc -g -Wall -Werror -c context.c

clean : 
	rm -rf libuthread.a *.o 
