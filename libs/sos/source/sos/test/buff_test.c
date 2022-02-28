/* mutex test... */
#include "oskit.h"
#include <string.h>

#define BUFFER_TEST_NUM 20
#define TEST_MEM_CLASS_ID 0x5a5a5acc
#define BUFF_SIZE 64*1024
#define MALLOC_SIZE (BUFF_SIZE/4)

#define MAX_SLOTS 32
static char *p[MAX_SLOTS];
static unsigned char pat[MAX_SLOTS];

/* buffer space */
static unsigned char space_class[BUFF_SIZE];

int main()
{
	int indx,res;
	int ii, kk,ix, randomsize;
	unsigned int size;
	unsigned int totsz;
	char randompat;
	char *c;
	unsigned char *buf;
	char str[32];

	/* OS initializing */
	os_init();

	printf("address=%08lx\n",(unsigned long)space_class);

	for(indx=0; indx<BUFFER_TEST_NUM; indx++) {
		res = buff_create_class(TEST_MEM_CLASS_ID+indx,space_class,BUFF_SIZE);
		if(res == ERROR) {
			printf("%d creation error\n",indx);
			break;
		}
		printf("TURN[%d]-------------------------------------\n",indx);
		totsz = 0;
		for(ii=0; ii<MAX_SLOTS; ii++) {
			randomsize = abs(random())%BUFF_SIZE+8;
			p[ii] = (char *)buff_alloc_data(TEST_MEM_CLASS_ID+indx,randomsize);
			if (p[ii] == NULL) break;
			pat[ii] = abs(random())%0xfe;
			memset(p[ii],pat[ii],randomsize);
			printf("\t[%-2d]Created %-10d pat=%02x addr=%08lx\n",ii,randomsize,pat[ii],(unsigned long)p[ii]);
			totsz += randomsize;
		}
		printf("OK creation %d with %d, totally %d,%d\n",indx,ii,totsz,BUFF_SIZE);
		for(kk=0; kk<ii; kk++) {
			buf = p[kk];
			size = buff_get_info(BUFF_GET_SIZE,buf);
			for(ix=0; ix<size; ix++) {
				if( *(unsigned char *)(buf + ix) != pat[kk] ) {
					printf("\t[%d]Crashed at %d size=%d \n",indx,kk,size);
					pat[kk] = 0xff;
					break;
				}
			}
			if(pat[kk]!=0xff) printf("\t[%d]Safe in %d,size=%d\n",indx,kk,size);
			buff_free_data(TEST_MEM_CLASS_ID+indx,(unsigned char *)p[kk]);
		}
		buff_delete_class(TEST_MEM_CLASS_ID+indx);
	}

	printf("Success..\n");

	return 1;
}
