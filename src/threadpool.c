#include "threadpool.h"

#include <stdlib.h>
#include <memory.h>
#include <pthread.h>

typedef struct threadtask_t {
	void (*func)(void*);
	void* args;
} threadtask;

typedef struct threadpool_t {
	int shutdown;

	int num_thread;
	pthread_t *worker_threads;

	int queue_size;
	int queue_start;
	int queue_end;
	int queue_count;
	threadtask *queue;
	pthread_mutex_t queue_lock;
	pthread_cond_t  queue_cond;

} threadpool;

void* threadpool_worker(void *args) {
	threadpool* pool = (threadpool*)args;
	int cur;
	threadtask task;

	while (1) {
		pthread_mutex_lock(&pool->queue_lock);

		while (pool->queue_count == 0 && pool->shutdown == 0) {
			pthread_cond_wait(&pool->queue_cond, &pool->queue_lock);
		}

		if (pool->shutdown != 0) {
			pthread_mutex_unlock(&pool->queue_lock);
			break;
		}
		cur = pool->queue_start;
		task.func = pool->queue[cur].func;
		task.args = pool->queue[cur].args;

		pool->queue_start = (cur+1) % pool->queue_size;
		pool->queue_count -= 1;

		(*(task.func))(task.args);

		pthread_mutex_unlock(&pool->queue_lock);
	}

	pthread_exit(NULL);
	return;
}

int threadpool_free(threadpool *pool) {
	int i, err = 0;

	if (pool == NULL)
		return threadpool_null_err;
	
	if (0 != pthread_mutex_lock(&pool->queue_lock))
		return threadpool_lock_err;

	do {
		pool->shutdown = 1;
		if (0 != pthread_cond_broadcast(&pool->queue_cond)) {
			err = threadpool_cond_err;
			break;
		}

		if (0 != pthread_mutex_unlock(&pool->queue_lock)) {
			err = threadpool_lock_err;
			break;
		}

		for (i = 0; i < pool->num_thread; i++) {
			pthread_join(pool->worker_threads[i], NULL);
		}
		pthread_mutex_unlock(&pool->queue_lock);
	} while(0);

	if (err == 0) {
		free(pool->worker_threads);
		free(pool->queue);
		pthread_mutex_destroy(&pool->queue_lock);
		pthread_cond_destroy(&pool->queue_cond);
	}
	return err;
}

threadpool* threadpool_init(int num_thread) {
	threadpool* pool;
	int i, queue_size;

	pool = (threadpool*)malloc(sizeof(threadpool));
	if (pool == NULL) 
		goto err;
	
	pool->shutdown = 0;

	pool->num_thread = num_thread;
	pool->worker_threads = (pthread_t*)malloc(sizeof(pthread_t) * num_thread);

	queue_size = num_thread * 2 + 1;
	pool->queue_size = queue_size;
	pool->queue_start = 0;
	pool->queue_end = 0;
	pool->queue_count = 0;
	pool->queue = (threadtask*)malloc(sizeof(threadtask) * queue_size);

	pthread_mutex_init(&pool->queue_lock, NULL);
	pthread_cond_init(&pool->queue_cond, NULL);

	for (i = 0; i < num_thread; i++) {
		pthread_create(&pool->worker_threads[i], NULL, threadpool_worker, (void*)pool);
	}

	return pool;
	
err:
	if (pool != NULL) {
		threadpool_free(pool);
	}
	return NULL;
}

int threadpool_add(threadpool *pool, void (*func)(void*), void* args) {
	int err = 0;
	int cur;

	if (0 != pthread_mutex_lock(&pool->queue_lock)) {
		return threadpool_lock_err;
	}

	do {
		if (pool->queue_size == pool->queue_count) {
			err = threadpool_full_err;
			break;
		}
		cur = pool->queue_end;
		pool->queue[cur].func = func;
		pool->queue[cur].args = args;
		pool->queue_end = (cur+1) % pool->queue_size;
		pool->queue_count += 1;

		if (0 != pthread_cond_signal(&pool->queue_cond)) {
			err = threadpool_cond_err;
		}
	} while(0);

	if (0 != pthread_mutex_unlock(&pool->queue_lock)) {
		err = threadpool_lock_err;
	}

	return err;
}
