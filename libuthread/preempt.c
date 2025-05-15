#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

static struct sigaction old_action;
static struct itimerval old;
static bool preempt_action = false;

void preempt_disable(void)
{
	if (preempt_action == false) {
		return;
	}
	sigset_t set;	
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);
}

void preempt_enable(void)
{
	if (preempt_action == false) {
                return;
        }
	sigset_t set;
	sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGVTALRM);
        sigprocmask(SIG_UNBLOCK, &set, NULL);
}

void preempt_start(bool preempt)
{
	if (preempt == false) {
		return false;
	}
	preempt_action = true;
	struct sigaction sa;
	sa.sa_handler = (void (*)(int)) uthread_yield; 
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGVTALRM, &sa, &old_action) < 0) {
		return;
	}
	struct itimerval timer; 
	timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = 1000000 / HZ;
        timer.it_value = timer.it_interval;

        if (setitimer(ITIMER_VIRTUAL, &timer, &old) < 0) {
                return;
        }
	
}

void preempt_stop(void)
{
	if (preempt_action == false) {
		return;
	}
	setitimer(ITIMER_VIRTUAL, &old, NULL);
        sigaction(SIGVTALRM, &old_action, NULL);
        preempt_action = false;
		
}

