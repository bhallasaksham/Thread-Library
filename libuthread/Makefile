all : libuthread.a

libuthread.a : queue.o
	ar rcs libuthread.a queue.o

queue.o : queue.c
	gcc -g -Wall -Werror -c queue.c

clean : 
	rm -rf libuthread.a *.o 
