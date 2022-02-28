#include <stdio.h>
#include <stdlib.h>
#include "dblib.h"

main()
{
	int res;
	unsigned int thread_id;

	os_init(); /* Base component initialize... */

	/* thread create */
	res = dbman_service_initialize(1092,"/tmp/test.dbf",NULL/*"/tmp/svrlog"*/);
	if(res < 0) {
		printf("DB manager initialization::failed\n");
		return;
	}

	while(1) {
		os_cond_verbose();
		buff_verbose(-1);
		sleep(1);
		printf("\n==================================\n");
	}
}

