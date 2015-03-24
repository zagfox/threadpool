#include <stdlib.h>
#include <stdio.h>

#include "threadpool.h"

void print(void* args) {
	fprintf(stderr, "hello\n");
}

int main() {
	//(*print)(NULL);
	threadpool *pool = threadpool_init(1);

	threadpool_add(pool, print, NULL);
	threadpool_add(pool, print, NULL);
	sleep(2);

	printf("begin free\n");
	threadpool_free(pool);

	return 0;
}
