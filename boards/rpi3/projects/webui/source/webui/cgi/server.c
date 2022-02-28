#include <fcgi_stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <json.h>
#include "svcproc.h"

static int reqcount = 0;

#define strXcmp(a,cstr)	strncasecmp((a),(cstr),strlen(cstr))

enum{
	GET_PROC = 0,
	PUT_PROC,
	POST_PROC,
	END_PROC
};

static svc_proc_t get_proc(char *cmd)
{
	cmdtbl_t *p = (cmdtbl_t *)&(cmdtbl[0]);

	while (p->cmd) {
		if (!strXcmp(cmd,p->cmd)) 
			return p->proc;
		++p;
	}
	return NULL;
}


int main(int argc, char *argv[])
{
	char * method;
	char * length;
	char * requri;
	char * type;
	char * charset;
	char * cmd, *path;
	char * uri;
	char fn[PATH_MAX+1];
	int payload_len;
	char *payload = NULL;
	int mode;
	int ix, count;
	char * param;
	#define MAXPARAM 6
	char * params[ MAXPARAM ];
	int ret;
	svc_proc_t proc;

	syslog(LOG_INFO,"Starting server.fcgi\n");

	while( FCGI_Accept() >= 0 ) { 
		syslog(LOG_INFO,"New request[%05x] Entered \n", ++reqcount );

		/* just three items */
		method = getenv("REQUEST_METHOD");
		length = getenv("CONTENT_LENGTH");
		requri = getenv("REQUEST_URI");
		
		/* check */
		if (!method || !length || !requri) {
			syslog(LOG_ERR, "request error method=<%s> legnth=<%s> requri=<%s>\n",
					method, length, requri);
			continue;
		}

		syslog(LOG_INFO, "REQ:: %s %s %s\n", method, requri, length);

		/*
		 *
		 * REQUEST_METHOD = "GET" / "PUT" depending on reqest 
		 * CONTENT_LENGTH = 0 - GET / 12+ - PUT depending on request
		 * REQUEST_URI = "/usr/sbin/server.fcgi?Command=xxxxx"
		 *
		 * --> This is how we deal with message...
		 *
		 */

		/* PUT ? GET ? - which one ?? */
		if (!strXcmp(method,"GET")) {
			mode = GET_PROC;
		} else if (!strXcmp(method, "PUT")) {
			mode = PUT_PROC;
		} else if (!strXcmp(method, "POST")) {
			mode = POST_PROC;
		} else {
			syslog(LOG_ERR, "Method=<%s> is none of GET/PUT/POST\n", method);
			continue;
		}

		/* payload dump */
		payload_len = strtol(length,NULL,10); /* payload length */
		if (payload_len >= 0) {
			payload = (char *)malloc( payload_len+1 );
			if (!payload) {
				syslog(LOG_ERR, "malloc( payload_len=%d ) failed\n",payload_len);
				continue;
			}
	
			/* Copying to a buffer */
			for(ix = 0; ix < payload_len; ix++) 
				payload[ ix ] = getchar();
			payload[ ix ] = 0x0;
		}

		/*
		 *
		 * Tokenizing command & parameters
		 *
		 */
		count = 0;
		param = strchr(requri,'?');
		if (!param) {	
			syslog(LOG_ERR, "No command && parameteres \n");
			continue;
		}
		++param;  /* command */
		params[ count++ ]  = param;
		while( (param = strchr(param, '&')) != NULL ) {
			*param = 0x0; /* tokenizing.. */
			params[ count++ ] = ++param;
			if (count >= (MAXPARAM-1)) {
				*param = 0x0;
				break;
			}
		}
	
		/* separating Command & Path */	
		cmd = NULL;
		path = NULL;
		for( ix = 0; ix < count; ix++ ) {
			if (!strncasecmp(params[ix],CMDSTR,strlen(CMDSTR))) {
				cmd = params[ ix ];
				cmd += strlen(CMDSTR);
				syslog(LOG_INFO, "Command[ %d ] = (%s)\n",ix,cmd);
			}
			if (!strncasecmp(params[ix],PATHSTR,strlen(PATHSTR))) {
				path = params[ ix ];
				path += strlen(PATHSTR);
				syslog(LOG_INFO, "Path[ %d ] = (%s)\n",ix,path);
			}
		}


		switch (mode) {
		case GET_PROC:
		case PUT_PROC:
			syslog(LOG_INFO,"%s CMD=(%s) (%s) (%s)\n", 
				mode == GET_PROC?"GET":
						( mode == PUT_PROC )? "PUT" : "POST", cmd, path?path:"EMPTY", payload );
			break;
		case POST_PROC:
			type = getenv("CONTENT_TYPE");
	
			/* tokenizing charset */
			charset = strchr(type,';');
			if ( !strncmp( ++charset, CHARSET, strlen(CHARSET)) )
				charset += strlen(CHARSET);

			syslog(LOG_INFO,"%s CMD=(%s) CONTENT=(%s) (%s)/(%s) \n", "POST", cmd, type, charset, path);

			/* file writing.... */
			{
			int amount;
			int fd;

			memset(fn,0,PATH_MAX+1);
			snprintf(fn,PATH_MAX,"%s/%s%c%s",TMPPATH,path,DELIMITER,charset);

			/* file upload */
			fd = open(fn,O_RDWR|O_CREAT,0666);
			if (fd < 0) {
				syslog(LOG_ERR,"file error %s",fn);
				free(payload);
				continue;
			}

			/* writing..*/
			amount = write(fd,payload,payload_len);
			syslog(LOG_INFO, "File Writing %d/%d in %s - %s",
					amount,payload_len,fn,amount == payload_len?"succeeded":"failed");
			close(fd);

			/* redirects path to file name */
			path = fn;
			}
			break;
		}

		/* service function searching.... */
		proc = get_proc(cmd);
		if (!proc) {
			syslog(LOG_ERR, "cmd=(%s) service not found!!\n", cmd);
			continue;
		}

		syslog(LOG_INFO, "CMD[%s] SERVICE --------\n",cmd);
		{
			ret = proc(mode,path,payload); /* call service */
			if (ret) {
				/* NOT OK !! */
			} else {
				/* OK !! */
			}
		}
		syslog(LOG_INFO, "CMD[%s] SERVICE DONE (%s) !! \n",cmd, !ret?"OK":"NOT OK");

		free(payload);
	}

	syslog(LOG_INFO,"--SERVER.fcgi\n");
}

