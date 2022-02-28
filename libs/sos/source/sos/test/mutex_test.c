/* mutex test... */
#include "oskit.h"

#define THR_NUM 15

typedef struct {
	unsigned int locked;
	unsigned int unlocked;
	unsigned int failed;
}mut_stat;

static mut_stat statis[THR_NUM];
static unsigned int mut0; /* global mutex */
static unsigned int valid_var = 0; /* shared variable */

static os_thrfunc_t mon(void *arg)
{
	int ii;

	while(1) {
		os_thread_sleep(1000000);
		printf("num   locked    unlocked    failed\n");
		for(ii=0; ii<THR_NUM; ii++) {
			printf("%3d    %d        %d          %d\n",
			       ii,
			       statis[ii].locked,
			       statis[ii].unlocked,
			       statis[ii].failed);
		}
	}
}

static os_thrfunc_t locker(void *arg)
{
	int counter = 0;
	int res;
	unsigned int id = os_thread_self();
	mut_stat *st = (mut_stat *)arg;

	while(1) {
		res = os_mutex_lock(mut0,20);
		if(res==OKAY) {
			st->locked++;
		}else{
			st->failed++;
			os_thread_sleep(10);
			continue;
		}
		++valid_var;
		if(valid_var != 1) {
			printf("\n\n\t VALIDITY FAILED... \n\n");
		}
		os_thread_sleep(10);
		--valid_var;
		os_mutex_unlock(mut0);
		st->unlocked++;
		os_thread_sleep(10000);
	}
}

/* delete function */
static os_delfunc_t delft(void *arg)
{
	printf("deleting..... %s\n",os_thread_name(os_thread_self()));
}

int main()
{
	unsigned int thrs[THR_NUM];
	unsigned int ii;
	unsigned char tname[32];

	/* OS initializing */
	os_init();

	mut0 = os_mutex_create("hello1");
	if(mut0 == ERROR) {
		printf("error in creating mut0\n");
		exit(0);
	}

	for(ii=0; ii<THR_NUM; ii++) {
		sprintf(tname,"thr%d",ii);
		memset((char *)&(statis[ii]),0,sizeof(mut_stat));
		thrs[ii] = os_thread_create(tname,(os_thrfunc_t)locker,(void *)&(statis[ii]),(os_delfunc_t)delft,0);
	}
	os_thread_create("monitor",(os_thrfunc_t)mon,NULL,NULL,0);

	/* join */
	while( 1 ) { os_thread_sleep(2000000); }

	return 0;
}

