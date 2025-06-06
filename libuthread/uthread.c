#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

#define RUNSTATE 0
#define READYSTATE 1
#define EXCITESTATE 2
#define BLOCKEDSTATE 3
#define STACKSIZE 100000

static queue_t queue_ready = NULL;
static struct uthread_tcb *idle = NULL;
static struct uthread_tcb *current = NULL;

struct uthread_tcb {
	uthread_ctx_t ctx;
	void *stack;
	int state;
};

struct uthread_tcb *uthread_current(void)
{
	return current;
}

void uthread_yield(void)
{
	struct uthread_tcb *next; 
	if (queue_dequeue(queue_ready, (void **)&next) < 0) {
		return;
	}
	if (current->state == RUNSTATE) {
		current->state = READYSTATE;
	}
	if (current != idle && current->state != EXCITESTATE) {
		queue_enqueue(queue_ready, current);
	}
	struct uthread_tcb *prev = current;
	current = next;
	current->state = RUNSTATE;
	uthread_ctx_switch(&prev->ctx, &current->ctx);
}

void uthread_exit(void)
{
	current->state = EXCITESTATE;
	if (current->stack != NULL) {
		uthread_ctx_destroy_stack(current->stack);
	}
	free(current);
	uthread_yield();
}

int uthread_create(uthread_func_t func, void *arg)
{
	struct uthread_tcb *tcb = malloc(sizeof(struct uthread_tcb));
	if (tcb == NULL) {
		return -1;
	}
	tcb->stack = uthread_ctx_alloc_stack();
	if (tcb->stack == NULL)  {
		free(tcb);
		return -1;
	}
	if (uthread_ctx_init(&tcb->ctx, tcb->stack, func, arg) < 0) {
		uthread_ctx_destroy_stack(tcb->stack);
		free(tcb);
		return -1;
	}
	tcb->state = READYSTATE;
	if (queue_enqueue(queue_ready, tcb) < 0) {
		uthread_ctx_destroy_stack(tcb->stack);
		free(tcb);
		return -1;
	}
	return 0;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	if (func == NULL) {
		return -1;
	}
	queue_ready = queue_create();
	if (queue_ready == NULL) {
		return -1;
	}
	idle = malloc(sizeof(struct uthread_tcb));
	if (idle == NULL) {
		return -1;
	}
	idle->stack = NULL;
	idle->state = RUNSTATE;
	current = idle;

	preempt_start(preempt);
	if (uthread_create(func, arg) < 0) {
		return -1;
	}
	while (queue_length(queue_ready) > 0) {
		uthread_yield();
	}

	preempt_stop(); 

	free(idle);
	queue_destroy(queue_ready);
	// idle = NULL;
	// current = NULL;
	// queue_ready = NULL;
	return 0;
	

}

void uthread_block(void)
{
	current->state = BLOCKEDSTATE;
	uthread_yield();
}


void uthread_unblock(struct uthread_tcb *uthread)
{
	if (uthread == NULL) {
		return;
	}
	uthread->state = READYSTATE;
	queue_enqueue(queue_ready, uthread);
}

