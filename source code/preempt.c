#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */

#define HZ 100

/* every time the timer_handler is called, the currently running thread yields */
void timer_handler (int signum) {
	uthread_yield();
}

/* using sigprocmask to disable SIGVTALRM signal */
void preempt_disable(void) {
	sigset_t newset;
	sigemptyset(&newset);
	sigaddset(&newset, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &newset, NULL);
}

/* using sigprocmask to enable SIGVTALRM signal */
void preempt_enable(void) {
	sigset_t newset;
	sigemptyset(&newset);
	sigaddset(&newset, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &newset, NULL);
}

void preempt_start(void) {
	/* configuring sigaction to catch SIGVTALRM signals */
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	/* timer_handler is called everytime timer triggers SIGVTALRM signal */
	sa.sa_handler = &timer_handler;
	sigaction(SIGVTALRM, &sa, NULL);

	/* configure the timer to expire 100 times per second */
	struct itimerval timer;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = HZ*100;
	setitimer(ITIMER_VIRTUAL, &timer, NULL);
}
