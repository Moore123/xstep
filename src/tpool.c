#include "pthreadpool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define DEBUG 1
func_t task_get(struct tasklist_t *tk_list, void **args)
{
	pthread_mutex_lock(&tk_list->lock);
	while (tk_list->taskcnt == 0)
		pthread_cond_wait(&tk_list->not_empty, &tk_list->lock);
	struct task_t *tmp;
	func_t func;
	tmp = tk_list->head->next;
	func = tmp->func;
	*args = tmp->args;
	free(tk_list->head);
	tk_list->head = tmp;
	tk_list->taskcnt--;
#if 0
	printf("taskcnt = %d\n", tk_list->taskcnt);
#endif
	pthread_cond_broadcast(&tk_list->not_full);
	pthread_mutex_unlock(&tk_list->lock);
	return func;
}

void task_add(struct tasklist_t *tk_list, func_t func, void *args)
{
	while( pthread_mutex_lock(&tk_list->lock) != 0 ) ;;
	while (tk_list->taskcnt == tk_list->task_max)
		pthread_cond_wait(&tk_list->not_full, &tk_list->lock);
	struct task_t *new = malloc(sizeof(*new));
	new->func = func;
	new->args = args;
	new->next = NULL;
	tk_list->tail->next = new;
	tk_list->tail = new;
	tk_list->taskcnt++;
#if 0
	printf("task_add sucess\n");
	func(NULL);
#endif
	pthread_cond_broadcast(&tk_list->not_empty);
	pthread_mutex_unlock(&tk_list->lock);
}

void *do_pthread(void *tp)
{
	func_t func;
	struct pthreadpool_t *tpool = tp;
	void *args;
	while (1) {
#if 0
		printf("do_pthread\n");
#endif
		func = tpool->tasklist.task_get(&tpool->tasklist, &args);
		(void)func(args);
	}
}

void tpool_wait(struct pthreadpool_t *tpool)
{
	int i;
	for (i = 0; i < tpool->maxpnum; i++)
		pthread_join(tpool->pids[i], NULL);
}

void tpool_init(struct pthreadpool_t *tpool, int pnum, int utm)
{
	tpool->pids = malloc(pnum * sizeof(pthread_t));
	tpool->maxpnum = pnum;
	tpool->tasklist.task_max = utm;
	pthread_mutex_init(&tpool->tasklist.lock, NULL);
	pthread_cond_init(&tpool->tasklist.not_empty, NULL);
	pthread_cond_init(&tpool->tasklist.not_full, NULL);

	tpool->tasklist.head = tpool->tasklist.tail = malloc(sizeof(*tpool->tasklist.head));
	tpool->tasklist.taskcnt = 0;
	tpool->tasklist.task_add = task_add;
	tpool->tasklist.task_get = task_get;

	tpool->do_pthread = do_pthread;
	tpool->tpool_init = tpool_init;

	int i;
	for (i = 0; i < pnum; i++) {
		int ret = pthread_create(&tpool->pids[i], NULL, tpool->do_pthread, tpool);
	}

}
