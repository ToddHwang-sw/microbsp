/* thread controller test */
#include "oskit.h"

#define MAX_THREADS 3

static os_thrfunc_t os_thread_func(void *arg)
{
	int counter = 0;
	int index = *(int *)arg;

	while(1) {
		debug_print("Test message %d...%d\n",index,++counter);
		os_thread_sleep(1000);
	}
}

int main()
{
	int ii;
	unsigned int thrs[MAX_THREADS];
	int counter = 0;
	OS_SYSPARAM sysp;

	/* reinit... */
	sysp.system_pool_size = 1024*1024;  /* 1 Mbytes */
	sysp.network_pool_size = 4*1024*1024; /* 4 Mbytes */
	os_reinit_sysparam(&sysp);

	/* debug level */
	os_set_debug_level(10);

	/* OS initializing */
	os_init();

	printf("thread delete test\n");

	/* threads */
	for(ii=0; ii<MAX_THREADS; ii++) {
		/* spawning deletable thread ... */
		thrs[ii] = os_thread_create("thr0",(os_thrfunc_t)os_thread_func,(void *)&ii,NULL,1);
		os_thread_sleep(10000);
	}

	/* join */
	while( 1 ) {
		os_thread_sleep(2000000);
		if( os_thread_isalive( &(thrs[counter]), 1 ) ) {
			printf("Now killing... thread (%08x)\n",thrs[counter]);
		#if 1
			os_thread_restartable(thrs[counter],0); /* cleanup restartable flag */
			os_thread_delete(thrs[counter]); /* deleting a thread */
		#else
			os_thread_restart(thrs[counter]); /* restarting a thread */
		#endif
		}else
			printf("Thread is dead\n");
		counter = (++counter) % MAX_THREADS;
	}

	return 0;
}

