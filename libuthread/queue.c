#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue {
	node_t *head;
	node_t *tail;
	int length;
};

struct node {
	void *data;
	struct node *next;
};

typedef struct node node_t;

queue_t queue_create(void)
{
	queue_t queue_o = malloc(sizeof(struct queue));
	if (queue_o == NULL) {
		return NULL;
	}
	queue_o->head = NULL;
	queue_o->tail = NULL;
	queue_o->length = 0;
	return queue_o;
}

int queue_destroy(queue_t queue)
{
	if (queue == NULL || queue->length > 0) {
		return -1; 
	}
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{ 
	if (queue == NULL || data == NULL) {
		return -1;
	}
	node_t *new_node = malloc(sizeof(node_t));
	new_node->data = data;
	new_node->next = NULL;
	if (queue->tail != NULL) {
		queue->tail->next = new_node;
	}
	else {
		queue->head = new_node;
	}
	queue->tail = new_node;
	queue->length++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if (queue == NULL || data == NULL || queue->length == 0) {
		return -1;
	}
	node_t *prev_head = queue->head;
	*data = prev_head->data;
	queue->head = prev_head->next;
	if (queue->head == NULL) {
		queue->tail = NULL;
	}
	free(prev_head);
	queue->length--;
}

int queue_delete(queue_t queue, void *data)
{
	if (queue == NULL || data == NULL) {
		return -1;
	}
	node_t *prev_node = NULL;
	node_t *curr_node = queue->head;
	while (curr_node != NULL) {
		if (curr->data == data) {
			if (prev_node == NULL) {
				queue->head = curr_node->next;
				if (queue->head == NULL) {
					queue->tail - NULL;
				}
			}
			else {
				prev->next = cur_node->next;
				if (curr_node == queue->tail) {
					queue->tail = prev_node;
				}			
			}
			free(curr_node);
			queue->length--;
			return 0;
		}
		prev_node = curr_node;
		curr_node = curr_node->next;
	}	
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	if queue == NULL || func == NULL) {
		return -1;
	}
	node_t *curr_node = queue->head;
	while (curr_node != NULL) {
		node_t *next = curr_node->next;
		func(queue, curr_node->data);
		curr_node = next;
	}
	return 0;
}

int queue_length(queue_t queue)
{
	if (queue == NULL) {
		return -1; 
	}
	return queue->length;
}

