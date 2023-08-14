#include <fcgi_stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include "svcproc.h"
#include "utils.h"

/*
 * + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + 
 *
 *
 *    
 *    S  E  R  V  I  C  E      F  U  N  C  T  I  O  N  s 
 *
 *
 *
 * + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + 
 */

/*
 *
 *  
 *
 */
#define BUFSIZE 1024
SVCFUNC_BEGIN(service_get,BUFSIZE)
{
	SVCFUNC_CLEAR(BUFSIZE)
	SVCFUNC_GETCMD()

	/* running~ */
	if (run_xcfgd( cmd , payload, SVCFUNC_CONTENT(jstr), BUFSIZE))
		return -1;

	syslog(LOG_INFO, "%s =  < %s >\n",__func__, jstr);
}
SVCFUNC_END
#undef BUFSIZE

/*
 *
 *  
 *
 */
#define BUFSIZE 2048
SVCFUNC_BEGIN(service_put,BUFSIZE)
{
	SVCFUNC_CLEAR(BUFSIZE)
	SVCFUNC_PUTCMD()

	syslog(LOG_INFO, "%s cmd=(%s) payload=(%s)\n",__func__, cmd, payload);

	/* running~ */
	if (run_xcfgd( cmd, payload, SVCFUNC_CONTENT(jstr), BUFSIZE))
		return -1;

	syslog(LOG_INFO, "%s = < %s >\n",__func__, jstr);
}
SVCFUNC_END
#undef BUFSIZE

/*
 *
 *  
 *
 */
#define BUFSIZE 128
SVCFUNC_BEGIN(file_upload,BUFSIZE)
{
	SVCFUNC_CLEAR(BUFSIZE)
	struct stat st;
	char *rfn;
	char xfn[PATH_MAX+1];
	char cmd[256];

	/* we just use path... */
	syslog(LOG_INFO, "%s cmd=(%s) path=(%s)\n",__func__, cmd, path);

	/* file statistics */
	if (stat(path,&st) == -1) {
		syslog(LOG_ERR, "%s file not found\n",path);
		return -1;
	}

	/* real path name */
	rfn = strchr(path,DELIMITER);

	memset( xfn, 0, PATH_MAX+1 );
	memcpy( xfn, path, rfn-path );
	syslog(LOG_INFO, "%s actual file name %s\n",__func__,xfn);

	/* converting a file... */
	memset( cmd, 0, 256 );
	snprintf( cmd, 255, "iconv -f UTF-8 -t ISO-8859-1 %s > %s", path, xfn );

	syslog(LOG_INFO, "%s command (%s) \n",__func__,cmd);

	system(cmd);

	/* deleting temporary file */
	remove(path);
}
SVCFUNC_END
#undef BUFSIZE


/*
 * 
 * C O M M A N D   T A B L E  
 *
 *  - This is referenced from server.c main routine. 
 *
 */
cmdtbl_t cmdtbl[] = {
	{ "GETSERVICE" , SVCFUNC_NAME(service_get) } , 
	{ "PUTSERVICE" , SVCFUNC_NAME(service_put) } , 
	{ "UPLOAD"     , SVCFUNC_NAME(file_upload) } , 
	{ NULL , NULL }
};

