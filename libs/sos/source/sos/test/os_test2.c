#include "oskit.h"

static unsigned int msgq1;

void os_thr_get_handler(void *arg)
{
	unsigned int id,ii;
	unsigned int msgq;
	OS_MESG *msg;

	msgq = *(unsigned int *)arg;

	id = os_thread_self();
	while ( 1 ) {
		msg = os_msgq_recv(msgq,0);
		printf("[%08x]recv[%08x][command=%08x]\n",id,msgq,OS_MESG_COMMAND(msg));
		os_mem_free(msg);
	}
}

void os_thr_put_handler(void *arg)
{
	unsigned int id,ii;
	unsigned int msgq;
	OS_MESG *msg;
	unsigned char s[20];

	msgq = *(unsigned int *)arg;

	memset(s,0,20);
	strcpy(s,"hello I");

	id = os_thread_self();
	while( 1 ) {
		os_thread_sleep(300000);
		msg = os_mem_alloc(sizeof(OS_MESG));
		if(!msg) {
			printf("error\n");
			exit(0);
		}
		os_msgq_message(msg,ii+0x12340000,s,strlen(s));
		os_msgq_send(msgq,msg);
		printf("[%08x]send[%08x][command=%5d]\n",id,msgq,msg->command);

		ii++;
	}
}

#define MY_MEM_CLASS 0x12345432

int main()
{
	int ii;
	unsigned int thrs[4];

	/* OS initializing */
	os_init();

	msgq1 = os_msgq_create("hello1",0,NULL);
	if(msgq1 == ERROR) {
		printf("error\n"); exit(0);
	}

	/* my pool */
	if( buff_create_class(MY_MEM_CLASS,NULL,1024*8) == ERROR ) {
		printf("error\n");
		exit(0);
	}

	thrs[0] = os_thread_create("thr0",(os_thrfunc_t)os_thr_get_handler,(void *)&msgq1,NULL,0);
	thrs[1] = os_thread_create("thr1",(os_thrfunc_t)os_thr_put_handler,(void *)&msgq1,NULL,0);
	thrs[2] = os_thread_create("thr2",(os_thrfunc_t)os_thr_get_handler,(void *)&msgq1,NULL,0);
	thrs[3] = os_thread_create("thr3",(os_thrfunc_t)os_thr_put_handler,(void *)&msgq1,NULL,0);

	sleep(10);

	os_msgq_delete(msgq1);

	os_thread_delete( thrs[0] );
	os_thread_delete( thrs[1] );
	os_thread_delete( thrs[2] );
	os_thread_delete( thrs[3] );

}

