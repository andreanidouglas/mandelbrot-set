#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"
#include "img.h"

typedef struct queue {
  struct queue_node_t *head;
  struct queue_node_t *tail;
} queue_t;

typedef struct mandel_t {
  int x;
  int y;
} mandel_t;

struct queue_node_t {
  mandel_t node;
  struct queue_node_t *next;
};

typedef struct mandel_prop_t {
  int width;
  int height;
  int bail_out;
  int max_iter;
} mandel_prop_t;

typedef struct mandel_buf_t {
  queue_t *q;
  int *result_field;
  mandel_prop_t prop;
  pthread_mutex_t *lock;
  int produce_end;
  ssize_t thread_num;
  ssize_t thread_pool[THREAD_COUNT];
} mandel_buf_t;

queue_t *initialize_queue();
void enqueue(queue_t *q, mandel_t w, pthread_mutex_t *lock);
mandel_t dequeue(queue_t *q, pthread_mutex_t *lock);
void destroy_queue(queue_t *q);
int is_empty(queue_t *q, pthread_mutex_t *lock);
