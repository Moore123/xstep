#ifndef _PTHREAD_POOL_H
#define _PTHREAD_POOL_H
#include <pthread.h>

typedef void (*func_t) (void *args);
struct task_t {
	struct task_t *next;
	func_t func;
	void *args;
};
struct tasklist_t {
	struct task_t *head;
	struct task_t *tail;
	pthread_mutex_t lock;
	pthread_cond_t not_empty;
	pthread_cond_t not_full;
	int taskcnt;
	int task_max;
	void (*task_add) (struct tasklist_t *, func_t func, void *args);
	 func_t(*task_get) (struct tasklist_t *, void **args);
};

struct pthreadpool_t {
	pthread_t *pids;
	int maxpnum;

	struct tasklist_t tasklist;

	void (*tpool_init) (struct pthreadpool_t * tpool, int pnum, int tm);
	void *(*do_pthread) (void *);
	void (*tpool_wait) (struct pthreadpool_t *);

};

extern void task_add(struct tasklist_t *, func_t , void *);
extern void *do_pthread(void *);
extern void tpool_wait(struct pthreadpool_t *);
extern void tpool_init(struct pthreadpool_t *, int , int );

#endif
