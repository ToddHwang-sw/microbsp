#include "oskit.h"

static unsigned int common_qbuff;
static int done;

#define gen_random() ((abs(rand())%4500))

#define TEST_BUFF_CLASS 0x0102abcd

static unsigned int id;

void os_thr_handler(void *arg)
{
	unsigned char *buff;
	unsigned int random_size;
	unsigned int res, csize;
	unsigned int counter = 0;
	unsigned int msg;

	id = os_thread_self();
	printf("THREAD ID = %08x\n",id);

	counter = 1;

	while( !done ) {
		random_size = gen_random(); /* random number */
		buff = (char *)malloc(random_size);
		if (!buff) {
			printf("malloc( %d ) - error\n",random_size);
			return ;
		}
		memset(buff,0,random_size);
		os_qbuff_put( common_qbuff, buff, random_size );
		printf("(%8s)(%5d)( %4d ) -----> BUFFER -----> ",os_thread_name(id),counter++,random_size);
		free(buff);
		os_thread_sleep(100000);

		random_size = gen_random(); /* random number */
		buff = (char *)malloc(random_size);
		if (!buff) {
			printf("malloc( %d ) - error\n",random_size);
			return ;
		}
		csize = os_qbuff_current_size( common_qbuff );
		res = os_qbuff_get( common_qbuff, buff, random_size );
		free(buff);
		printf(" ( %4d/%7d=%4d ) \n",random_size, csize, res);
	}

	printf("THREAD KILLED !!\n");
}

static unsigned int mydelf(void *arg);

/* test run procedure */
static void test_procedure(void *baseaddr,unsigned int sz)
{
	unsigned int thr;
	unsigned int res;

	printf("BUFFER_CREATE( %d ) \n", sz);

	done = 0;

	res = buff_create_class(TEST_BUFF_CLASS,baseaddr,sz);
	if(res == ERROR) {
		printf("buff_create_class::error\n");
	}

	/* default and default */
	common_qbuff = os_qbuff_create("myqueue",TEST_BUFF_CLASS,NULL); /* Qbuffer creating */
	if(common_qbuff == ERROR) {
		printf("error\n");
		exit(0);
	}

	/* thread create */
	thr = os_thread_create("thr0",(os_thrfunc_t)os_thr_handler,NULL,mydelf,0);

	/* 3 seconds */
	sleep(5);

	printf("\n\n\n\nTHREAD DELETE... \n\n\n");

	done = 1; /* signalling */

	/* thread delete */
	os_thread_delete(thr);
}

/* user space area.. */
static unsigned char user_base[8192+512];

static unsigned int mydelf(void *arg)
{
	/* statically declared variable */
	printf("\n\ndelete function\n");

	/* q delete */
	if( os_qbuff_delete(common_qbuff) == ERROR ) {
		printf("error in os_qbuff_delete() \n");
	}
	printf("deleting a queue\n");

	/* buffer delete */
	if( buff_delete_class( TEST_BUFF_CLASS ) == ERROR ) {
		printf("error in buff_delete_class() \n");
	}
	printf("deleting a class\n");

	return OKAY;
}

#define TEST_ROUND 4
/* test vector */
static struct {
	void *baddr;
	unsigned int sz;
} __test_vector_st[TEST_ROUND] =
{
	{NULL,8192},           /* buffer(system-2048), (system memory/default delfuntion) */
	{user_base,8192},      /* buffer(user-2048),   (system memory/default delfuntion) */
	{NULL,8192},           /* buffer(system-2048),  (user memory/default delfuntion) */
	{user_base,8192},      /* buffer(user-1024), (user memory/default delfuntion) */
};

int main()
{
	int ii;

	/* OS initializing */
	os_init();

	/* test... */
	for(ii=0; ii<TEST_ROUND; ii++) {

		printf("------------ ROUND %d --------------\n", ii+1 );
		test_procedure( __test_vector_st[ii].baddr,
		                __test_vector_st[ii].sz);

		printf("------------ ROUND %d END !!! ------\n", ii+1 );
	}

	/* join */
	while( 1 ) {
		os_thread_verbose();
		os_mutex_verbose();
		sleep(1);
	}

}

