#include "mandelbrot.h"

queue_t *initialize_queue()
{
	queue_t *q;

	q = (queue_t *) malloc(sizeof(queue_t *));

	return q;
}

void enqueue(queue_t * q, mandel_t * w, pthread_mutex_t * lock)
{
	struct queue_node_t *aux;

	pthread_mutex_lock(lock);

	if (q->head == NULL) {
		q->head =
		    (struct queue_node_t *)
		    malloc(sizeof(struct queue_node_t *));
		q->head->next = NULL;
		q->head->node = w;
		q->tail = q->head;
	} else {
		aux =
		    (struct queue_node_t *)
		    malloc(sizeof(struct queue_node_t *));
		aux->next = NULL;
		aux->node = w;
		q->tail->next = aux;
		q->tail = aux;
	}

	pthread_mutex_unlock(lock);
}

mandel_t *dequeue(queue_t * q, pthread_mutex_t * lock)
{
	struct queue_node_t *aux;
	mandel_t *ret_val;

	if (q->head != NULL) {
		pthread_mutex_lock(lock);
		ret_val = q->head->node;
		aux = q->head;
		q->head = q->head->next;
		pthread_mutex_unlock(lock);
		free(aux);
		return (ret_val);
	}
	return NULL;

}

int is_empty(queue_t * q)
{
	return (q->head == NULL);
}

void destroy_queue(queue_t * q)
{
	struct queue_node_t *aux, *next;

	aux = q->head;

	while (aux != NULL) {
		next = aux->next;
		free(aux);
		if (next != NULL)
			aux = next;
	}
	free(q);
}
