#include "oskit.h"

#define TEST_MEM_CLASS  0x1234abcd

typedef struct {
	unsigned long data;
	unsigned int size;
	unsigned int class;
	unsigned char val;
}__attribute__((aligned)) msg_st;

static unsigned int mesgq;

unsigned int allocs = 0; /* statistics */

static void * OS_MEM_ALLOC(int sz)
{
	void * newmem = os_mem_alloc(sz);
	// printf("alloc=%08x\n",(unsigned long)newmem);
	return newmem;
}

static void OS_MEM_FREE(void *mem)
{
	// printf("free=%08x\n",(unsigned long)mem);
	os_mem_free(mem);
}

static os_thrfunc_t os_thr_malloc_handler(void *arg)
{
	unsigned int id,ii,ix;
	unsigned char *data;
	unsigned char val;
	msg_st *m;
	OS_MESG *msg;
	unsigned int size,classnum;
	unsigned int res;

	ii = 0;
	id = os_thread_self();
	while(1) {
		ii += 1;
		classnum = 1;
		size = abs(rand()%1024) & ~0x3;
		val = abs(rand()%0xff);
		data = (unsigned char *)OS_MEM_ALLOC(size);
		if(!data) {
			printf("(%08x)OS_MEM_ALLOC(%d,%d)::error\n",id,classnum,size);
			// os_msgq_flush(mesgq);
			os_thread_sleep(1000000);
			continue;
		}
		m = (msg_st *)OS_MEM_ALLOC(sizeof(msg_st));
		if(!m) { printf("OS_MEM_ALLOC(sizeof(msg_st))::error\n");
			     OS_MEM_FREE((void *)data);
			     // os_msgq_flush(mesgq);
			     os_thread_sleep(1000000);
			     continue;}
		msg = (OS_MESG *)OS_MEM_ALLOC(sizeof(OS_MESG));
		if(!msg) {
			printf("(%08x)OS_MEM_ALLOC(OS_MESG)::error\n",id);
			OS_MEM_FREE((void *)data);
			OS_MEM_FREE((void *)m);
			// os_msgq_flush(mesgq);
			os_thread_sleep(1000000);
			continue;
		}
		memset(data,val,size);
		m->data = (unsigned long)data;
		m->size = size;
		m->class = classnum;
		m->val = val;

		/* statistics */
		allocs += (size + sizeof(msg_st) + sizeof(OS_MESG));

		printf("(%s)inserted with %p %d %x\n",os_thread_name(os_thread_self()),data,size,val);

		for(ix=0; ix<size; ix++) {
			if(data[ix] != val) {
				printf("broken memory... at %d %x::%x\n", ix, data[ix], val);
			}
		}

		os_msgq_message(msg,0x1234abcd,(void *)m,sizeof(void *));
		if(os_msgq_send(mesgq,msg)==ERROR) {
			printf("(%s)(%08x)os_msgq_send::error\n",os_thread_name(id),id);
			OS_MEM_FREE((void *)data);
			OS_MEM_FREE((void *)m);
			OS_MEM_FREE((void *)msg);
			os_msgq_flush(mesgq);
			// os_thread_sleep(1000000);
			continue;
		}
		os_thread_sleep(500000); /* 500 milliseconds */
	}

	os_thread_delete(id);
}

static os_thrfunc_t os_thr_free_handler(void *arg)
{
	unsigned int id,ii,ix;
	unsigned char *blk;
	unsigned char val;
	OS_MESG *msg;
	msg_st *m;
	unsigned int size,classnum;

	id = os_thread_self();
	ii = 0;
	while(1) {
		++ii;
		msg = os_msgq_recv(mesgq,0);
		if(!msg) {
			if( os_errno_get() == E_NODATA ) {
				printf("(%s)timeout\n",os_thread_name(id));
			}
			printf("(%s)os_msgq_recv()::error\n",os_thread_name(id));
			continue;
		}
		m = (msg_st *)OS_MESG_DATA(msg);
		if(!m) {
			printf("NULL data\n");
			continue;
		}
		blk = (char *)(m->data);
		size = m->size;
		classnum = m->class;
		val = m->val;
		if(!blk) {
			printf("null data\n");
			continue;
		}
		printf("(%s)retrived with %p %d %x\n",os_thread_name(os_thread_self()),blk,size,val);
		for(ix=0; ix<size; ix++) {
			if(blk[ix] != val) {
				printf("broken memory... at %d %x::%x\n", ix, blk[ix], val);
			}
		}

		printf("(%s)deleted with %p %p %p\n",os_thread_name(os_thread_self()),msg,m,blk);
		OS_MEM_FREE((void *)blk);
		OS_MEM_FREE((void *)msg);
		OS_MEM_FREE((void *)m);
		allocs -= (size + sizeof(msg_st) + sizeof(OS_MESG));
	}

	os_thread_delete(id);
}

static unsigned int usr_delete_func(void *msg)
{
	unsigned long maddr = *(unsigned long *)msg;
	OS_MESG *m;
	msg_st *p;

	/* message */
	m = (OS_MESG *)maddr;
	if(!m) {
		printf("error\n");
		exit(0);
	}
	p = (msg_st *)OS_MESG_DATA(m);

	if(p) {
		printf("(%s)deleting ",os_thread_name(os_thread_self()));
		if(p->data) {
			printf("data=%lx ",p->data);
			OS_MEM_FREE((void *)(p->data));
		}
		if(p) {
			printf("msg=%p ",p);
			OS_MEM_FREE(p);
		}
		OS_MEM_FREE(m);
		printf("container=%p ",m);
		printf("\n");
	}

	return 0;
}

#define TEST_MEM 0x12341234

int main()
{
	int ii;
	unsigned int res;
	unsigned int thrs[6];
	region_info_t t;

	/* OS initializing */
	os_init();

	/* mesgq = os_msgq_create("MSGQ",0,usr_delete_func); */
	mesgq = os_msgq_create("MSGQ",0,NULL);
	if(mesgq==ERROR) {
		printf("os_msgq_create(ERROR)\n");
		exit(0);
	}

	/* threads */
	thrs[0] = os_thread_create("thr0",(os_thrfunc_t)os_thr_free_handler,NULL,NULL,0);
	thrs[1] = os_thread_create("thr1",(os_thrfunc_t)os_thr_malloc_handler,NULL,NULL,0);
	thrs[2] = os_thread_create("thr2",(os_thrfunc_t)os_thr_free_handler,NULL,NULL,0);
	thrs[3] = os_thread_create("thr3",(os_thrfunc_t)os_thr_malloc_handler,NULL,NULL,0);
	thrs[4] = os_thread_create("thr4",(os_thrfunc_t)os_thr_free_handler,NULL,NULL,0);
	thrs[6] = os_thread_create("thr5",(os_thrfunc_t)os_thr_malloc_handler,NULL,NULL,0);

	/* join */
	while( 1 ) {
		os_thread_sleep(100);
		/* obtaining... */
		os_mem_resinfo(&t);
		/* cleaning,... */
		printf("size = %10d,, blocks=%10ld\r",allocs,t.ordblks);
	}

	return 0;
}

