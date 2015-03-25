
typedef enum {
	threadpool_ok       = 0,
	threadpool_lock_err = -1,
	threadpool_cond_err = -2,
	threadpool_shut_err = -3,
	threadpool_full_err = -5,
	threadpool_null_err = -10
} threadpool_err;

typedef struct threadpool_t threadpool;

/*
 * @brief create a thread pool, with num_thread workers
 * @param num_thread Number of workers
 * @return A new thread pool, with workers running
 */
threadpool* threadpool_init(int num_thread, int queue_size);

/*
 * @brief add a task to threadpool
 * @param pool The pool to be added
 * @param func The function
 * @param args The arguments
 * @return The status code defined in threadpool_err
 */
int threadpool_add(threadpool *pool, void (*func)(void*), void* args);

/*
 * @brief Terminiate all work and delete a thread pool
 * @param pool The threadpool to be delete
 * @param force Whether to force shutdown
 * @return The status code defined in threadpool_err
 */
int threadpool_free(threadpool *pool, int force);
