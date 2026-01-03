#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include "svcproc.h"

static long get_usec_time( void ) 
{
	struct timeval cTime;
	gettimeofday(&cTime, NULL);

	return cTime.tv_sec * (int)1e6 + cTime.tv_usec;
}

int run_xcfgd( char *param , char *payload, char * retstr , int len )
{
	char tmpfile[ 512 ];
	FILE *tmpfp;
	char command[ 1024 ];
	struct stat st;
	int ret;
	int fd;
	int amount, r_amount;

	/* temporary file name */
	srand( get_usec_time() );
	memset(tmpfile,0,128);
	snprintf(tmpfile,128-strlen(TMPPATH)-16, "%s/xcfgd_%d.tmp",TMPPATH,rand());

	/* temporary file creation */
	tmpfp = fopen(tmpfile,"w");
	if (!tmpfp) {
		syslog(LOG_ERR, "Temporary file (%s) creation error\n",tmpfile);
		return -1;
	}
	fclose(tmpfp);

	/* command execution */
	memset(command,0,1024);
	if (payload) {
		sprintf(command, "%s %s %s %s >& %s" , ENVP, EXECUTOR, param, payload, tmpfile ); /* PUT */
	} else {
		sprintf(command, "%s %s %s >& %s" , ENVP, EXECUTOR, param, tmpfile ); /* GET */
	}

	syslog(LOG_INFO, "Command is \"%s\" \n",command);

	/* running command... */
	system(command);

	/* dump result */
	if (stat(tmpfile,&st)) {
		syslog(LOG_ERR, "stat(%s) failed\n",tmpfile);
		fclose(tmpfp);
		goto err_return;
	}

	/* dump file */
	fd = open(tmpfile,O_RDONLY);
	if (fd < 0) {
		syslog(LOG_ERR, "open(%s) failed\n",tmpfile);
		goto err_return;
	}
	amount = len<st.st_size?len:st.st_size;
	syslog(LOG_INFO, "Reading file (%s) as much as %d bytes\n", tmpfile, amount);
	r_amount = read(fd, retstr, amount );
	close(fd);

	if (r_amount != amount) {
		syslog(LOG_ERR, "read(.. %d) = %d failed\n",amount,r_amount);
		goto err_return;
	}

	syslog(LOG_INFO, "Temporary file %s deleted.\n",tmpfile);

	remove(tmpfile);
	return 0;

err_return:
	remove(tmpfile);
	return -1;
}

