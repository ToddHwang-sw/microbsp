#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>
#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "timer.h"
#include "common.h"

#define timer_func_in(fmt, args ...)         (void)0
#define timer_func_out(fmt, args ...)        (void)0

#define CREATED_MASK        0x54494D45UL

/*
 *
 * T O O L K I T      F U N C T I O N S
 *
 */
static inline const char *get_cur_time(void)
{
	static char buf[64];
	time_t t;
	struct tm *tm;
	struct timeval tv;

	time(&t);
	tm = localtime(&t);
	gettimeofday(&tv, NULL);

	sprintf(buf, "%02d:%02d:%02d:%03d",
	        tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec/1000));
	return buf;
}

static inline const char *get_date(time_t *t)
{
	static char buf[64];
	struct tm *tm;

	tm = localtime(t);
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
	        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return buf;
}

static inline const char *get_cur_date(void)
{
	time_t t;

	time(&t);
	return get_date(&t);
}

#define time2ms(time_ptr)   (((time_ptr)->tv_sec * 1000) + ((time_ptr)->tv_usec / 1000))

static inline int gettimemsofday(void)
{
	struct timeval time;

	gettimeofday(&time, NULL);
	return time2ms(&time);
}

/*
 *
 * M  A  I  N         F U N C T I O N S
 *
 */
typedef struct pthread_timer_s {
	unsigned tt_inited;
	pthread_t tt_thread;
	pthread_mutex_t tt_lock;
	pthread_cond_t tt_cond;

	struct list_head tt_head;
	timer_obj_t         *tt_active;

} pthread_timer_t;

static pthread_timer_t pthread_timer;

static struct timespec *get_timespec_ms(struct timespec *ts, int timeout_ms)
{
	#define nsec_per_sec    (1000*1000*1000)
	#define ps_timeval2timespec(tv,ts)  \
	((ts)->tv_sec = (tv)->tv_sec, (ts)->tv_nsec = (tv)->tv_usec * 1000)
	struct timeval tv;
	int sec, nsec;

	sec = timeout_ms/1000;
	nsec = (timeout_ms%1000) * 1000 /*usec*/ * 1000 /*nsec*/;

	gettimeofday(&tv, NULL);
	ps_timeval2timespec(&tv, ts);
	ts->tv_sec += sec;
	ts->tv_nsec += nsec;

	if (ts->tv_nsec >= nsec_per_sec) {
		ts->tv_sec++;
		ts->tv_nsec -= nsec_per_sec;
	}
	return ts;
}

static struct timespec *cmp_earlier_timespec(struct timespec *ts1, struct timespec *ts2)
{
	if (ts1->tv_sec == ts2->tv_sec) {
		if (ts1->tv_nsec < ts2->tv_nsec)
			return ts1;
	}
	else if (ts1->tv_sec < ts2->tv_sec)
		return ts1;
	return ts2;
}

#if defined(TIMER_ASSERT)
static void time_assert(struct timespec *ts)
{
	struct timespec curr_ts;

	get_timespec_ms(&curr_ts, 0);

	assert(cmp_earlier_timespec(&curr_ts, ts) == ts);
}
#endif

static void *timer_thread(void *thr_data)
{
	pthread_timer_t *ptt = &pthread_timer;
	struct list_head *head = &ptt->tt_head;
	timer_obj_t *to;
	void *data;
	void (*callback)(void *data);
	int ret;

	timer_func_in();

	while (1) {
		pthread_mutex_lock(&ptt->tt_lock);

		/*
		   start_timer is called, cond-lock is released,
		   but at that time,
		   if stop_timer is called before wake up cond-wait, the list becomes empty.
		   Thus, we use while loop instead of if.
		 */
		while (list_empty(head))
			ret = pthread_cond_wait(&ptt->tt_cond, &ptt->tt_lock);

		to = list_entry(head->next, timer_obj_t, to_list);
		list_del_init(&to->to_list);

		ptt->tt_active = to;
		ret = pthread_cond_timedwait(&ptt->tt_cond, &ptt->tt_lock, &to->to_ts);
		ptt->tt_active = NULL;
		if (ret == ETIMEDOUT && to->to_active) {
			#if defined(TIMER_ASSERT)
			time_assert(&to->to_ts);
			#endif
			callback = to->to_callback;
			data = to->to_data;
			pthread_mutex_lock(&to->to_lock);
		}
		else
			callback = NULL;
		pthread_mutex_unlock(&ptt->tt_lock);

		if (callback) {
			callback(data);
			pthread_mutex_unlock(&to->to_lock);
		}
	}

	timer_func_out();
	return NULL;
}

int init_timer(timer_obj_t *timer, char *name, void (*callback)(void *), void *data)
{
	pthread_timer_t *ptt = &pthread_timer;

	if (!ptt->tt_inited) {
		ERR("Timer module hasn't been initialized!!\n");
		return -1;
	}

	DBG("\n");

	if (!timer || !callback || !data) {
		ERR("Invalid parameter\n");
		return -1;
	}

	pthread_mutex_init(&timer->to_lock, NULL);
	INIT_LIST_HEAD(&timer->to_list);
	memset(timer->name,0,32);
	strncpy(timer->name,name,32-1);
	timer->to_callback = callback;
	timer->to_data = data;
	timer->to_active = 0;
	timer->to_created = CREATED_MASK;

	return 0;
}

int start_timer(timer_obj_t *timer, int expire_milisec)
{
	pthread_timer_t *ptt = &pthread_timer;
	struct list_head *head = &ptt->tt_head;
	timer_obj_t *pos;
	struct timespec ts;
	int ret = 0, inserted = 0;

	if (!ptt->tt_inited) {
		ERR("Timer module hasn't been initialized!!\n");
		return -1;
	}

	DBG("name=%s %dmsec\n",timer->name, expire_milisec);

	get_timespec_ms(&ts, expire_milisec);

	pthread_mutex_lock(&ptt->tt_lock);
	if (timer->to_created != CREATED_MASK) {
		ERR("The timer was not to_created or deleted.\n");
		goto out;
	}
	if (pthread_self() != ptt->tt_thread)
		pthread_mutex_lock(&timer->to_lock);

	memcpy(&timer->to_ts, &ts, sizeof(ts));
	timer->to_active = 1;
	timer->to_caller = __builtin_return_address(0);

	if (list_empty(head))
		list_add(&timer->to_list, head);
	else {
		list_for_each_entry(pos, head, to_list) {
			if (pos != timer) {
				if (cmp_earlier_timespec(&timer->to_ts, &pos->to_ts) == &timer->to_ts) {
					if (list_empty(&timer->to_list))    /*It isn't in list*/
						list_add(&timer->to_list, &pos->to_list);
					else
						list_move(&timer->to_list, &pos->to_list);
					inserted = 1;
					break;
				}
			}
		}

		if (!inserted) {
			if (list_empty(&timer->to_list))    /*It isn't in list*/
				list_add_tail(&timer->to_list, head);
			else
				list_move_tail(&timer->to_list, head);
		}
	}

	assert(!list_empty(&timer->to_list));

	if (ptt->tt_active) {
		if (ptt->tt_active == timer)
			pthread_cond_signal(&ptt->tt_cond);
		else if (list_entry(head->next, timer_obj_t, to_list) == timer) {/*is head?*/
			if (cmp_earlier_timespec(&timer->to_ts, &ptt->tt_active->to_ts)
			    == &timer->to_ts) {
				list_add(&ptt->tt_active->to_list, &timer->to_list);
				pthread_cond_signal(&ptt->tt_cond);
			}
		}
	}
	else if (list_entry(head->next, timer_obj_t, to_list) == timer) /*is head?*/
		pthread_cond_signal(&ptt->tt_cond);

	if (pthread_self() != ptt->tt_thread)
		pthread_mutex_unlock(&timer->to_lock);
out:
	pthread_mutex_unlock(&ptt->tt_lock);
	return ret;
}

int stop_timer(timer_obj_t *timer)
{
	pthread_timer_t *ptt = &pthread_timer;
	int ret = 0;

	if (!ptt->tt_inited) {
		ERR("Timer module hasn't been initialized!!\n");
		return -1;
	}

	DBG("name=%s\n",timer->name);

	pthread_mutex_lock(&ptt->tt_lock);
	if (timer->to_created != CREATED_MASK)
		goto out;

	if (pthread_self() != ptt->tt_thread)
		pthread_mutex_lock(&timer->to_lock);

	timer->to_active = 0;

	if (ptt->tt_active == timer)
		pthread_cond_signal(&ptt->tt_cond);
	else
		list_del_init(&timer->to_list);

	if (pthread_self() != ptt->tt_thread)
		pthread_mutex_unlock(&timer->to_lock);
out:
	pthread_mutex_unlock(&ptt->tt_lock);
	return ret;
}

int del_timer(timer_obj_t *timer)
{
	int ret = 0;

	DBG("name=%s\n",timer->name);

	ret = stop_timer(timer);
	timer->to_created = 0;

	return ret;
}

int pending_timer(timer_obj_t *timer)
{
	pthread_timer_t *ptt = &pthread_timer;
	struct list_head *head = &ptt->tt_head;
	int res;

	if (!ptt->tt_inited)
		return -1;

	pthread_mutex_lock(&ptt->tt_lock);
	res = list_empty(head) ? 0 : 1;
	pthread_mutex_unlock(&ptt->tt_lock);
	return res;
}

int timer_module_init(void)
{
	pthread_timer_t *ptt = &pthread_timer;

	INIT_LIST_HEAD(&ptt->tt_head);

	pthread_mutex_init(&ptt->tt_lock, NULL);
	pthread_cond_init(&ptt->tt_cond, NULL);
	ptt->tt_inited = 1;

	pthread_create(&ptt->tt_thread, NULL, timer_thread, (void *) NULL);

	return 0;
}

void timer_module_deinit(void)
{
	pthread_timer_t *ptt = &pthread_timer;
	pthread_t thread;

	if ((thread = ptt->tt_thread)) {
		ptt->tt_thread = (pthread_t) NULL;
		pthread_cancel(thread);
		pthread_join(thread, NULL);
	}

	pthread_mutex_destroy(&ptt->tt_lock);
	pthread_cond_destroy(&ptt->tt_cond);
	ptt->tt_inited = 0;
}

#ifdef __cplusplus
}
#endif

