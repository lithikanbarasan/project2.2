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
#define STACKSIZE 100000

static queue_t queue_ready = NULL;
static struct uthread_tcb *idle = NULL;
static struct uthread_tcb *current = NULL;

struct uthread_tcb {
	void *stack;
	int state;
};

struct uthread_tcb *uthread_current(void)
{
	/* TODO Phase 2/3 */
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
		uthread_ctx_destroy_stack(current->stack, STACKSIZE);
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
	tcb->stack = uthread_ctx_alloc_stack(STACKSIZE);
	if (!tcb->stack) {
		free(tcb);
		return -1;
	}
	if (uthread_ctx_init(&tcb->ctx, tcb->stack, STACKSIZE, func, arg) < 0) {
		uthread_ctx_destroy_stack(tcb->stack, STACKSIZE);
		free(tcb);
		return -1;
	}
	tcb->state = READYSTATE;
	if (queue_enqueue(queue_ready, tcb) < 0) {
		uthread_ctx_destroy_stack(tcb->stack, STACKSIZE);
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
	if (uthread_create(func, arg) == -1) {
		return -1;
	}
	while (queue_length(queue_ready) > 0) {
		uthread_yield();
	}
	free(idle);
	queue_destroy(queue_ready);
	// idle = NULL;
	// current = NULL;
	// queue_ready = NULL;
	return 0;
	

}

void uthread_block(void)
{
	/* TODO Phase 3 */
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/* TODO Phase 3 */
}

