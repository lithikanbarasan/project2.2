#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "private.h"
#include "sem.h"

struct semaphore {
	size_t count; 
	queue_t queue_block;
};

sem_t sem_create(size_t count)
{
	sem_t sem = malloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}
	sem->count = count;
	sem->queue_block = queue_create();
	if (sem->queue_block == NULL) {
		free(sem);
		return NULL;
	}
	return sem; 
}

int sem_destroy(sem_t sem)
{
	if (sem == NULL || queue_length(sem->queue_block) > 0) {
		return -1;
	}
	queue_destroy(sem->queue_block);
	free(sem);
	return 0;
}

int sem_down(sem_t sem)
{
	if (sem == NULL) {
		return -1;
	}
	while(sem->count == 0) {
		queue_enqueue(sem->queue_block, uthread_current());
		uthread_block();
	}
	sem->count--;
	return 0;
}

int sem_up(sem_t sem)
{
	if (sem == NULL) {
		return -1;
	}
	struct uthread_tcb *next = NULL;
	if (queue_dequeue(sem->queue_block, (void **)&next) == 0) {
		uthread_unblock(next):
	}
	else {
		sem->count++;
	}
	return 0;
}

