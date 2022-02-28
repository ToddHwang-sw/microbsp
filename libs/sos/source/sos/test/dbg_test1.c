/* debugging message test... */
#include "oskit.h"

static os_thrfunc_t os_dbg_thread(void *arg)
{
	int counter = 0;

	while(1) {
		debug_print("(%08x)Test message %d...\n",os_thread_self(),++counter);
		os_thread_sleep(1000);
	}
}

int main()
{
	unsigned int thrs0;
	unsigned int thrs1;

	/* OS initializing */
	os_init();

	thrs0 = os_thread_create("thr0",(os_thrfunc_t)os_dbg_thread,NULL,NULL,0);
	thrs1 = os_thread_create("thr1",(os_thrfunc_t)os_dbg_thread,NULL,NULL,0);

	/* join */
	while( 1 ) { os_thread_sleep(2000000); }

	return 1;
}

