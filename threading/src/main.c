#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREADS 8
#define QUEUE_SIZE 100

typedef struct {
  void (*fn)(void *arg);
  void *arg;
} task_t;

typedef struct {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t threads[THREADS];
  task_t task_queue[QUEUE_SIZE];
  int queued;
  int queue_front;
  int queue_back;
  int stop;
} threadpool_t;

void thread_pool_init(threadpool_t *pool) {
  pool->queued = 0;
  pool->queue_front = 0;
  pool->queue_back = 0;
  pool->stop = 0;
  pthread_mutex_init(&pool->lock, NULL);
  pthread_cond_init(&pool->notify, NULL);
  for (int i = 0; i < THREADS; i++) {
    pthread_create(&pool->threads[i], NULL, thread_function, pool);
  }
}
