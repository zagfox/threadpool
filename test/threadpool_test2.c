#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

#include <pthread.h>
#include "threadpool.h"

#define THREAD 96
#define QUEUE  256

pthread_mutex_t lock;
int done = 0;

void dummy_task(void* args) {
	usleep(100000);
	pthread_mutex_lock(&lock);
	++done;
	pthread_mutex_unlock(&lock);
}

int main() {
	int i;
	pthread_mutex_init(&lock, NULL);

	threadpool *pool = threadpool_init(THREAD, QUEUE);

	for (i = 0; i < THREAD; i++) {
		threadpool_add(pool, dummy_task, NULL);
	}

	printf("begin free\n");
	threadpool_free(pool, 2);

	printf("done: %d\n", done);
	assert(done == THREAD);

	return 0;
}
