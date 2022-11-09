#include "mandelbrot.h"

queue_t *initialize_queue()
{
    queue_t *q;

    q = (queue_t *) malloc(sizeof(queue_t ));

    return q;
}

void enqueue(queue_t * q, mandel_t * w, pthread_mutex_t * lock)
{
    struct queue_node_t *aux;

    (void) lock;


    if (q->head == NULL) {
	q->head = (struct queue_node_t * )
	    malloc(sizeof(struct queue_node_t ));
	q->head->next = NULL;
	q->head->node = w;
	q->tail = q->head;
    } else {
	aux = (struct queue_node_t * )
	    malloc(sizeof(struct queue_node_t ));

	aux->next = NULL;
	aux->node = w;
	q->tail->next = aux;
	q->tail = aux;
    }

}

mandel_t *dequeue(queue_t * q, pthread_mutex_t * lock)
{

    (void) lock;
    struct queue_node_t *aux;
    mandel_t *ret_val;
    //pthread_mutex_lock(lock);

    if (q->head == NULL) {
	ret_val = NULL;
    } else if (q->head == q->tail) {
	aux = q->head;
	ret_val = aux->node;
	q->head = NULL;
	q->tail = NULL;
    } else {
	aux = q->head;
	ret_val = aux->node;
	q->head = aux->next;
    }
    //pthread_mutex_unlock(lock);
    return ret_val;

}

int is_empty(queue_t * q, pthread_mutex_t * lock)
{
    int ret = 0;
    pthread_mutex_lock(lock);
    if (q->head == NULL)
	ret = 1;
    pthread_mutex_unlock(lock);

    return (ret);
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
