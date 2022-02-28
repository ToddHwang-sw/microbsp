#ifndef TIMER_H_07282008
#define TIMER_H_07282008

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sys/time.h>
#include "list.h"

typedef struct timer_obj_s {
	char name[32];
	struct list_head to_list;
	unsigned to_created;
	pthread_mutex_t to_lock;
	struct timespec to_ts;
	int to_active;
	void *to_data;
	void (*to_callback)(void *data);

	/*debug info*/
	void *to_caller;
} __attribute__((aligned)) timer_obj_t;

extern int timer_module_init(void);
extern void timer_module_deinit(void);

extern int init_timer(timer_obj_t *timer, char *name, void (*callback)(void *), void *data);
extern int start_timer(timer_obj_t *timer, int expire_milisec);
extern int stop_timer(timer_obj_t *timer);
extern int del_timer(timer_obj_t *timer);
extern int pending_timer(timer_obj_t *timer);

#ifdef __cplusplus
}
#endif

#endif

