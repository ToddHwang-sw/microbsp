#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#include "malloc.h"
#include "xmlmem.h"

/* This is the name of shell processor */
static char sproc[ 1024 ] = {0,};

int check_shell_proc( char * procname, const char * url )
{
	FILE * in;
	char cmd[ 1024 ];
	static char xbuf[ 32 ];
	struct stat st;

	if ( stat( procname, &st ) < 0 ) {
		ERR("SHELL PROCESSOR %s DOES NOT EXIST.\n", procname);
		return 1;
	}

	memset(cmd, 0, 1024);
	sprintf(cmd,"%s check %s", procname, url );
	if ( !(in = popen(cmd,"r")) ) {
		ERR("SHELL PROCESSOR %s CANNOT HANDLE %s COMMAND.\n",procname,url);
		return 1;
	}

	/* name set */
	if (sproc[0] == 0x0) {
		memset(sproc,0,1024);
		memcpy(sproc,procname,strlen(procname));
		DBG("PROC NAME SETUP (%s) \n",sproc);
	}

	/* processing...*/
	memset( xbuf, 0, 32 );
	while( fgets(xbuf,sizeof(xbuf)-1,in) != NULL );

	pclose(in);

	/* deleting '\n' */
	xbuf[ strlen(xbuf) - 1 ] = 0x0;

	/*
	 *
	 * 'check' command in shell processor should return either "YES" or "NO".
	 *
	 */
	return !strncmp(xbuf,"YE",strlen("YE")) ? 0 : 1;
}

#define __MAX_SHELL_CMD_LEN__ 2048

char * run_shell_proc( const char *url )
{
	FILE * in;
	char cmd[ 1024 ];
	char *res;

	if (sproc[0] == 0x00)
		return NULL;

	memset(cmd, 0, 1024);
	sprintf(cmd,"%s run %s", sproc, url );
	if ( !(in = popen(cmd,"r")) ) {
		ERR("SHELL PROCESSOR %s CANNOT RUN %s COMMAND.\n",sproc,url);
		return NULL;
	}

	res = (char *)__os_malloc(__MAX_SHELL_CMD_LEN__);
	if (!res) {
		ERR("__os_malloc( 2048 ) failed\n");
		return NULL;
	}
	memset(res, 0, __MAX_SHELL_CMD_LEN__);

	/* processing...*/
	while( fgets(res,__MAX_SHELL_CMD_LEN__-1,in) != NULL );

	pclose(in);

	/* deleting '\n' */
	res[ strlen(res) - 1 ] = 0x0;

	DBG("SHELL PROC RUN(%s) -> (%s) \n",url,res);

	return res;
}
