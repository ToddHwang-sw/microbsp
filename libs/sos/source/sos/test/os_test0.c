#include "oskit.h"

char *f, *s, *t;

#define MAX_ALLOCS (1024*8)
typedef struct {
	unsigned char *addr;
	unsigned int sz;
}_malloc_st;
static _malloc_st addrbuf[MAX_ALLOCS];

int main()
{
	int ii,cnt,sz;

	/* OS initializing */
	os_init();

	while(1) {
		/* allocation */
		printf("phase allocation \n");
		cnt = 0;
		while(1) {
			sz = abs(rand()%4096);
			addrbuf[cnt].addr = (char *)os_mem_alloc(sz);
			if(addrbuf[cnt].addr==NULL) {
				break;
			}
			addrbuf[cnt].sz = sz;
			printf("alloc...(%08lx)%d\n",(unsigned long)addrbuf[cnt].addr,sz);
			if( ++cnt >= MAX_ALLOCS ) break;
		}

		printf("phase deallocation \n");
		/* free... */
		for(ii=0; ii<cnt; ii++) {
			os_mem_free(addrbuf[ii].addr);
			printf("freeing... (%08lx)%d\n",(unsigned long)addrbuf[ii].addr,addrbuf[ii].sz);
		}

	} /* while() */

	return 0;
}

