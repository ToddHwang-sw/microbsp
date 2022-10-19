#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/limits.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdarg.h>
#include <syslog.h>
#include "scew.h"
#include "dmt.h"
#include "jansson.h"
#include "malloc.h"
#include "xmlmem.h"
#include "common.h"
#include "microhttpd.h"
#include "xml.h"
#include "timer.h"
#include "xprint.h"
#include "storage.h"
#include "shellproc.h"

/* synchronization... - max delay 180 seconds ( 3 minutes ) */
static char xcfg_sync_tmpfile[128];

/* last 6 bytes */
#define TEMPFN_TEMPLATE "/var/tmp/xcfgXXXXXXX"
#define _MAX_WAIT_TIME_   180 

/*
 *
 *
 * P U B L I C      V A R I A B L E S
 *
 *
 */

int sys_loglevel = LOGLEVEL_DEFAULT; /* loglevel */
int sys_rmem = XML_MEMSZ; /* 4 Mbytes */
int sys_logsupport = 0; /* no syslog support */
int readonlyMode = 0; /* when device name was not given ... */

/*
 *
 *
 * P U B L I C      F U N C T I O N S
 *
 *
 */
void logfunc(const char *fmt, ...)
{
	int size = 0;
	char *p = NULL;
	va_list ap;

	/* size determine */
	va_start(ap,fmt);
	size = vsnprintf(p, size, fmt, ap);
	va_end(ap);

	if (size <= 0)
		return;

	++size;
	p = (char *) os_malloc( size );
	if (!p)
		return;

	memset(p,0x0,size);

	/* real buffer printout ... */
	va_start(ap,fmt);
	size = vsnprintf(p, size-1, fmt, ap);
	va_end(ap);

	if (size < 0) {
		os_free(p);
		return;
	}

	/* syslog/console output */
	if (sys_logsupport)
		syslog(LOG_INFO,"%s\n",p);
	else
		printf("%s\n",p);

	os_free(p);
}

/*
 *
 *
 * P R I V A T E    D E F I N I T I O N S
 *
 *
 */

/* server info */
struct xcfg_info {

	int done;               /* termination flag */

	/* HTTP connection */
	#define DEFAULT_HTTP_PORT 8083
	unsigned short http_port; /* HTTP port */
	int client_timeout;
	#define DEFAULT_MAIN_IDLE 5
	int main_idle;
	#define DEFAULT_SYNC_EXPIRE (1000)
	int sync_expire;
	#define DEFAULT_MAXUSER (8)
	int max_user;
	int cur_user;
	char    *       shellproc;

	char * cafile;
	char * keyfile;

	struct MHD_Daemon *httpd; /* HTTP daemon */

	/* service thread */
	pthread_mutex_t mut;

	/* random temporary file name */
	struct timeval rand_tv;

	/* sync timer */
	timer_obj_t timer;

	/* XML information */
	char path[ PATH_MAX + 8 ];             /* temporary XML */
	char dpath[ PATH_MAX + 8 ];             /* device path */
	scew_parser     *parser;
	scew_tree       *tree;

	/* temporary data for XML handling */
	scew_list       *list;
	scew_element    *element;
	char            *contents;

	/* json string */
	json_t          *json_res;

} __attribute__((aligned));

/*
 *
 *
 * P R I V A T E    V A R I A B L E S
 *
 *
 */

static struct xcfg_info * xcfg;

/* storage information */
static storage_info stor_info =
{
	.hnum = DEFAULT_STORAGE_HEADER_NUM,
	.bnum = DEFAULT_STORAGE_DATA_NUM,
	.bsize = DEFAULT_STORAGE_DATA_SIZE >> 10,
};

/*
 *
 *
 * P R I V A T E    T O O L K I T   F U N C T I O N S
 *
 *
 */
static char * rand_string(char *str, size_t size)
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (size) {
		--size;
		for( size_t n = 0; n < size; n++ ) {
			int key = rand() % (int)(sizeof charset - 1);
			str[n] = charset[key];
		}
		str[size] = '\0';
	}
	return str;
}

/*
 *
 *
 * P R I V A T E    F U N C T I O N S
 *
 *
 */
static void sync_handler( void * arg )
{
	struct xcfg_info * xcfg = (struct xcfg_info *)arg;
	int rc;

	if (!xcfg)
		return;

	pthread_mutex_lock( &xcfg->mut );

	/* Writing to a file... */
	if( config_xml_write_tree( xcfg->tree, xcfg->path ) )
		goto __exit_sync_handler;

	/* Update storage space */
	if (xcfg->dpath[0] != 0) 
		xml_storage_update( xcfg->dpath, xcfg->path );

	/* Removing the previous tree */
	config_xml_exit( xcfg->parser, xcfg->tree );

	/* Reloading new file */
	rc = config_xml_init( xcfg->path, &xcfg->parser, &xcfg->tree );
	if ( rc < 0 )
		ERR("config_xml_init -> failed \n");

__exit_sync_handler:
	pthread_mutex_unlock( &xcfg->mut );
}

static void update_sync_timer( struct xcfg_info *xcfg )
{
	timer_obj_t *timer;

	if (!xcfg)
		return;

	timer = (timer_obj_t *)&(xcfg->timer);

	stop_timer( timer ); /* deleting previous timer */
	start_timer( timer, xcfg->sync_expire ); /* rewind a timer again */
}

static void make_access_info_file( short port )
{
	FILE *fp;
	char name[PATH_MAX+1];

	memset(name,0,PATH_MAX+1);

	//strcpy(name,TEMP_XML_PREFIX);
	sprintf(name, TEMP_XML_PREFIX "/%d",getpid());
	strcat(name,"/");
	strcat(name,PORT_FILE);

	fp = fopen(name,"w+");
	if (fp) {
		fprintf(fp,"%d", port);
		fclose(fp);
	} else
		ERR("%s cannot be created\n",name);
}

/*
 *
 *
 * H T T P    S E R V E R    C A L L B A C K S
 *
 *
 */
static int AcceptCallback(void * cls,
                          const struct sockaddr *addr,
                          unsigned int len)
{
	struct sockaddr_in a;

	/*
	 * "cls" is NULL !!
	 */

	/* make temporal copy */
	memset((char *)&a, 0, sizeof(struct sockaddr_in));
	memcpy((char *)&a, (char *)addr, sizeof(struct sockaddr_in));

	DBG("HTTP ACCEPT [CLIENT] %s\n",inet_ntoa( a.sin_addr ) );

	return MHD_YES;
}

static int DefaultHandlerCallback(void *cls,
                                  struct MHD_Connection *connection,
                                  const char *url,
                                  const char *method,
                                  const char *version,
                                  const char *updata, size_t *updata_size,
                                  void **unused)
{
	struct MHD_Response *response = NULL;
	unsigned int status_code = 200; /* OK */
	unsigned int syncreq;
	const char *purl = url+1;
	char * rbuf;
	char * respmesg;
	#define __error_string__ "{\"error\": \"error\"}"

	DBG("HTTP HANDLE URL=%s METHOD=%s VER=%s DATA=%s\n", url, method, version, updata ? updata : "" );

	pthread_mutex_lock( &xcfg->mut );

	syncreq = 0; /* synchronization request */

	/* purl keeps ... */

	if (strcmp(url,"/") == 0) {
		ERR("ERROR URL \n");
		pthread_mutex_unlock( &xcfg->mut );
		return MHD_NO;
	}

	if (strcasecmp(method,"GET") == 0) {
		/* retrieve value */
		xcfg->json_res = config_xml_retrieve( xcfg->tree, purl );
		if( !xcfg->json_res ) {
			/* check hidden node */
			if ( !check_shell_proc( xcfg->shellproc, purl ) ) {
				if( (rbuf = run_shell_proc( purl )) == NULL ) {
					status_code = 302; /* URI not found */
					respmesg = (char *) os_malloc( strlen( __error_string__ ) );
					strcpy( respmesg, __error_string__ );
				} else {
					respmesg = rbuf;
				}
			} else {
				ERR("HTTP URL(%s) is unknown\n",url);
				status_code = 302; /* URI not found */
				respmesg = (char *) os_malloc( strlen( __error_string__ ) );
				strcpy( respmesg, __error_string__ );
			}
		} else
			respmesg =
				json_dumps( xcfg->json_res, JSON_PRESERVE_ORDER );
		if (!respmesg) {
			pthread_mutex_unlock( &xcfg->mut );
			return MHD_NO;
		}
	} else
	if ((strcasecmp(method,"POST") == 0) ||
	    (strcasecmp(method,"PUT") ==0)) {
		json_t *jobj = NULL;
		#define TMPFILELEN 64
		char tfn[TMPFILELEN];  /* long enough ~ */

		/* check readonly mode */
		if (readonlyMode) { 
			ERR("READ-ONLY MODE\n");
			pthread_mutex_unlock( &xcfg->mut );
			return MHD_NO;
		}

		if (!strlen(updata)) {
			ERR("HTTP PUT/POST NULL VALUE\n");
			pthread_mutex_unlock( &xcfg->mut );
			return MHD_NO;
		}

		memset(tfn,0,TMPFILELEN);

		/* making temporary file name */
		{
			int len = sprintf(tfn,TEMP_XML_PREFIX "/%d/",getpid());
			rand_string(((char *)tfn+len),(size_t)((sizeof(tfn)-4)-len));
		}

		FILE * fp = (FILE *)fopen(tfn,"w+");
		if (!fp) {
			ERR("json temp file (%s) creation error\n",tfn);
			status_code = 302;
			respmesg = (char *) os_malloc( strlen( __error_string__ ) );
			strcpy( respmesg, __error_string__ );
		} else {
			fprintf(fp,"%s",updata);
			fclose(fp);
			json_error_t jerr;
			jobj = json_load_file(tfn,0,&jerr);
			if (!jobj) {
				ERR("json object cannot be built\n");
				status_code = 302; /* URI not found */
				respmesg = (char *) os_malloc( strlen( __error_string__ ) );
				strcpy( respmesg, __error_string__ );
			}
			unlink( tfn );
		}

		if (status_code == 200) {
			char newval[ 128 ];
			if (!config_xml_json_value( jobj, newval )) {
				xcfg->json_res = config_xml_set( xcfg->tree,
				                                 (const char *)purl, (const char *)newval );
				/* update value */
				respmesg = json_dumps( jobj, JSON_PRESERVE_ORDER );
				syncreq = 1; /* synchronization request - persistent storage update */
			}
		}
		if( jobj )
			json_decref( jobj );

	}else{
		ERR("Unknown method\n");
		pthread_mutex_unlock( &xcfg->mut );
		return MHD_NO;
	}	

	/*
	 * NOTICE - string shorter than 64 has been processed
	 * with scrambled garbage - probably fault in HTTP library.
	 */
	if (strlen(respmesg) < 64)
		respmesg = (char *)os_realloc(respmesg,64);

	DBG("HTTP URL RES=(%s)\n",respmesg);

	/* response message */
	response = MHD_create_response_from_buffer(
		strlen(respmesg), (void *)respmesg, MHD_RESPMEM_MUST_COPY);
	MHD_queue_response( connection, status_code, response );

	/* remove json */
	config_xml_remove( xcfg->json_res );
	json_decref( xcfg->json_res );

	/* remove response */
	if( respmesg )
		os_free( respmesg );

	/* delete response */
	if( response )
		MHD_destroy_response(response);

	if (syncreq)
		update_sync_timer( xcfg );

	pthread_mutex_unlock( &xcfg->mut );
	return MHD_YES;
}

static void ConnectionCallback(void * cls,
                               struct MHD_Connection *connection,
                               void **socket_context,
                               enum MHD_ConnectionNotificationCode toe)
{
	static int started = MHD_NO;

	switch (toe)
	{
	case MHD_CONNECTION_NOTIFY_STARTED:
		DBG("HTTP CONNECTION START (%d/%d)\n",
		    xcfg->cur_user, xcfg->max_user);
		if (MHD_NO != started)
			return;
		if (xcfg->max_user <= (xcfg->cur_user+1)) {
			DBG("HTTP CONNECTION MAX USER REACHED\n");
			return;
		}
		++xcfg->cur_user;
		started = MHD_YES;
		*socket_context = &started;
		break;
	case MHD_CONNECTION_NOTIFY_CLOSED:
		DBG("HTTP CONNECTION CLOSE (%d/%d)\n",
		    xcfg->cur_user, xcfg->max_user);
		if (MHD_YES != started)
			return;
		if (&started != *socket_context)
			return;
		if (xcfg->cur_user == 0) {
			DBG("HTTP CONNECTION USER COUNT CRASH!!\n");
			xcfg->cur_user = 1; /* CREEPY ~ */
		}
		--xcfg->cur_user;
		*socket_context = NULL;
		started = MHD_NO;
		break;
	default:
		DBG("HTTP CONNECTION UNKNOWN\n");
		break;
	}
}

static void * LogCallback(void *cls,
                          const char *uri,
                          struct MHD_Connection *connection)
{
	if (uri)
		DBG("HTTP URI %s \n",uri);

	return (void *)NULL;
}

static void CompletionCallback(void *cls,
                               struct MHD_Connection *connection,
                               void **con_cls,
                               enum MHD_RequestTerminationCode toe)
{
	DBG("HTTP COMPLETION\n");

	if ( (toe != MHD_REQUEST_TERMINATED_COMPLETED_OK) &&
	     (toe != MHD_REQUEST_TERMINATED_CLIENT_ABORT) &&
	     (toe != MHD_REQUEST_TERMINATED_DAEMON_SHUTDOWN) )
		return;

	__os_free (*con_cls);
	*con_cls = NULL;
}

static __inline__ void usage(char *pgm)
{
	printf("usage)\n\n");
	printf("%s -f (basic XML file) | -d (media storage) | [ -m (new XML file for merge) ] | [ -l loglevel ] \n",pgm);
	printf("   -s <standalone mode>| -r (system memory - Kbytes) | -x (max user) \n");
	printf("   -n <header block>   | -b (data block)    | -e (data block suze - Kbytes) \n");
	printf("   -y <syslog support> \n");
	printf("e.g.) \n");
	printf("# %s -f config.xml -d /var/tmp/testdb                      \n", pgm );
	printf("\t\t        ---- config.xml in current folder will be maintained in /var/tmp/testdb. \n");
	printf("# %s -f config.xml -d /dev/mtd8                            \n", pgm );
	printf("\t\t		---- config.xml in current folder will be maintained in /dev/mtd8.  \n");
	printf("# %s -f config.xml -d /dev/mtd9 -n 2 -b 2 -e 128           \n", pgm );
	printf("\t\t		---- total header 2, total data block 2, each data block size 128kbytes\n");
	printf("# %s -f config.xml -d /var/tmp/testdb -m config.2.xml      \n", pgm );
	printf("\t\t        ---- config.xml + config.2.xml --> config.xml and \n");
	printf("\t\t             config.xml will be maintained in /var/tmp/testdb. \n");
	printf("# %s -f config.xml -d /var/tmp/testdb -l 2 \n", pgm );
	printf("\t\t        ---- loglevel 0~3 : 0:silent/1:Debug/2:Timer/3:Memory(Noisy) \n");
}

/*
 *
 *
 * S Y S T E M     S I G N A L    F U N C T I O N S
 *
 *
 */
/* generic daemonize function */
static void daemonize()
{
	pid_t pid;
	
	pid = fork();
	if (pid < 0) {
		ERR("deaemonize::error\n");
		exit(0);
	}

	if (pid == 0) {
		/* child */
	} else {
		int maxcnt=_MAX_WAIT_TIME_; 

		while (maxcnt--) {
			int fd;
			if( (fd = open(xcfg_sync_tmpfile,O_RDONLY,0666)) > 0 ) {
				close(fd);
				DBG("TEMPORARY FILE - SYNC : %s DELETE\n",xcfg_sync_tmpfile);
				remove(xcfg_sync_tmpfile);
				MSG("XCFGD DAEMONIZED\n");
				break;
			}
			sleep(1); /* 1 second delay */
		}
		exit(0);
	}

	setsid();
	signal(SIGHUP, SIG_IGN);

#if 0
	if ( (pid = fork()) != 0 )
	       exit(0);
#endif

	chdir("/");
	umask(0);
}

/* signal function - HUP */
static void xcfg_hup( int sig )
{
	/* keep relaying */
	if (sig == SIGHUP)
		signal( SIGHUP, xcfg_hup );
}

/* signal function - TERM/INT/QUIT */
static void xcfg_trap( int sig )
{
	DBG("Trapped and now suiciding.. \n");
	xcfg->done = 1; /* now.. close this program */
}

#ifdef _USE_HTTPS_
/* Test Certificate */
static char cert_pem[65536];
static char key_pem[65536];
const char * cert_s/C=US/ST=CA/L=Irvine/O=Unixboy/OU=RU/CN=127.0.0.1ubj = "/C=US/ST=CA/L=Irvine/O=Unixboy/OU=RU/CN=127.0.0.1";
#endif 

/*
 *
 *
 * M A I N          F U N C T I O N S
 *
 *
 */

int main(int argc,char *argv[])
{
	int ret;
	int pend;
	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"config", required_argument, 0, 'f'},
		{"device", required_argument, 0, 'd'},
		{"port", required_argument, 0, 'p'},
		{"log", required_argument, 0, 'l'},
		{"standalone", required_argument, 0, 's'},
		{"resource", required_argument, 0, 'r'},
		{"maxuser", required_argument, 0, 'x'},
		{"headers", required_argument, 0, 'n'},
		{"blocks", required_argument, 0, 'b'},
		{"blocksize", required_argument, 0, 'e'},
		{NULL, 0, 0, 0}
	};
	int c, option_index;
	char *xmlfn  = NULL; /* permanent XML : eg. "/etc/config.xml" */
	char *devnm  = NULL; /* device area   : eg. "/dev/mtd5" */
	char *txmlfn = NULL; /* temporary XML : eg. "/var/tmp/config.xml"  */
	short xport = 0;
	int loglevel;
	int standalone = 0;
	int rmem = 0;
	int maxuser = DEFAULT_MAXUSER;
	int merged = 0;

	/* temporary file */
	memset( xcfg_sync_tmpfile, 0, 128 );
	strncpy( xcfg_sync_tmpfile, TEMPFN_TEMPLATE , 128-1 );
	mktemp( xcfg_sync_tmpfile );

	DBG("TEMPORARY FILE - SYNC : %s\n",xcfg_sync_tmpfile);
		
	optind = 0;
	while (1) {
		option_index = 0;
		c = getopt_long(argc, argv, "hsyf:d:p:l:r:x:n:b:e:",long_options, &option_index);
		if (c == -1)
			break;

		/* check */
		switch(c) {
		case 'h':
			usage( argv[0] );
			exit(0);
		case 's':
			standalone = 1;
			DBG("STANDADLONE MODE\n");
			break;
		case 'd':
			devnm = strdup( optarg );
			DBG("DEV : %s\n",devnm);
			break;
		case 'f':
			xmlfn = strdup( optarg );
			DBG("XML1 : %s\n",xmlfn);
			break;
		case 'p':
			xport = atoi( optarg );
			DBG("PORT : %d\n",xport);
			break;
		case 'l':
			loglevel = atoi( optarg );
			if ((loglevel >= 0) && (loglevel <= LOGLEVEL_NUM))
				sys_loglevel = loglevel;
			DBG("LOGLEVEL: %d\n",sys_loglevel);
			break;
		case 'r':
			rmem = atoi( optarg );
			if (((rmem >> 10) >= 1) && ((rmem >> 10) <= 10))
				sys_rmem = rmem << 10;
			DBG("MEMORY: %d\n",sys_rmem);
			break;
		case 'x':
			maxuser = atoi( optarg );
			if (!(maxuser >= 1 && maxuser < 16))
				maxuser = DEFAULT_MAXUSER;
			DBG("MAX USER: %d\n",maxuser);
			break;
		case 'y':
			/* sys_logsupport = 1;
			DBG("SYSLOG SUPPORT !!\n"); */
			break;
		case 'n':  /* header block number */
			stor_info.hnum = atoi( optarg );
			if (!(stor_info.hnum >= 2 && stor_info.hnum <= 4))
				stor_info.hnum = DEFAULT_STORAGE_HEADER_NUM;
			DBG("HEADER BLOCK: %d\n",stor_info.hnum);
			break;
		case 'b':  /* header block number */
			stor_info.bnum = atoi( optarg );
			if (!(stor_info.bnum >= 2 && stor_info.bnum <= 4))
				stor_info.bnum = DEFAULT_STORAGE_DATA_NUM;
			DBG("DATA BLOCK: %d\n",stor_info.bnum);
			break;
		case 'e':  /* header block number */
			stor_info.bsize = atoi( optarg );
			if (!(stor_info.bsize >= 128 && stor_info.bsize <= 1024))
				stor_info.bsize = DEFAULT_STORAGE_DATA_SIZE;
			DBG("DATA BLOCK SIZE: %dKbytes\n",stor_info.bsize);
			break;
		}
	}

	/* check */
#if 0
	if (!xmlfn || !devnm) {
#else
	if (!xmlfn) {
#endif
		usage( argv[0] );
		return 1;
	}

	if (!devnm)
		readonlyMode = 1;

	if (!standalone) {
		daemonize();
		/* signal HUP */
		signal( SIGHUP, xcfg_hup );
	}

	/* signal trap */
	signal( SIGQUIT, xcfg_trap );
	signal( SIGTERM, xcfg_trap );
	signal( SIGINT, xcfg_trap );

	{
		char ldir[PATH_MAX+1];
		int status;

		memset(ldir,0,PATH_MAX+1);
		sprintf(ldir,TEMP_XML_PREFIX "/%d",getpid());
		DBG("TEMPORARY DIRECTORY = (%s)\n",ldir);
		status = mkdir(ldir,0776);
		if (status)
			return 1;
	}

	if ( !devnm ) {
		txmlfn = (char *)os_malloc(strlen(xmlfn)+1);
		if (!txmlfn) {
			ERR("os_malloc(%d)::failed\n",strlen(xmlfn)+1);
			return 1;
		}
		memset(txmlfn,0,strlen(xmlfn)+1);
		strcpy(txmlfn, xmlfn);
		DBG("NULL DEVICE - READONLY MODE \n");
	} else {
		/* device type - MTD ? FILE ? */
		int devtype = strstr(devnm,"/dev/") ? STOR_MTD : STOR_FILE;

		/* Storage Maintenance */
		xml_storage_select( devtype );
		xml_storage_open( devnm, xmlfn, &stor_info );
		if ((txmlfn = (char *)xml_storage_validate( xmlfn )) == NULL) {
			ERR("xml_storage_validate( %s ) ::failed\n",xmlfn);
			return 1;
		}

		merged = 0;
		if( !config_xml_merge( txmlfn, xmlfn ) ) {
			DBG("XML has been merged into %s \n", txmlfn);
			merged = 1;
		}
	}

	xcfg = (struct xcfg_info *)malloc( sizeof(struct xcfg_info) );
	if (!xcfg) {
		ERR("malloc( sizeof(struct xcfg_info) )::failed\n");
		return 1;
	}

	memset( (char *)xcfg, 0, sizeof(struct xcfg_info) );
	strcpy( xcfg->path, txmlfn );
	if (!devnm) 
		xcfg->dpath[0] = 0;
	else
		strcpy( xcfg->dpath, devnm );
	xcfg->parser = NULL;
	xcfg->tree =   NULL;
	xcfg->list =   NULL;
	xcfg->element =  NULL;
	xcfg->contents = NULL;
	xcfg->done = 0;
	xcfg->max_user = maxuser;
	xcfg->cur_user = 0;

	/* freeeing up */
	if (devnm) free( devnm );
	if (xmlfn) free( xmlfn );
	if (txmlfn) __os_free(txmlfn);

	/* timer system init */
	timer_module_init();

	/* processing merging first... */
	/* make XML trees get ready */
	ret = config_xml_init( xcfg->path, &xcfg->parser, &xcfg->tree );
	if ( ret < 0 ) {
		ERR("config_xml_init -> failed \n");
		return 1;
	}

	/* SYSLOG */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"operation/syslog/en" );
	if( xcfg->json_res ) {
		int enable; char _en[32];
		enable = !config_xml_json_value( xcfg->json_res, _en ) ? atoi(_en) : 0;
		config_xml_remove( xcfg->json_res );
		if ( !enable )
			sys_logsupport = 0;
		else
			sys_logsupport = 1;
	}
	MSG("SYSLOG SUPPORT : %s\n", sys_logsupport ? "YES" : "NO" );

	/* HTTP port */
	if (xport) {
		xcfg->http_port = xport; /* HTTP network port */
	} else {
		xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/httpport" );
		if( xcfg->json_res ) {
			char _hport[32];
			xcfg->http_port =
				!config_xml_json_value( xcfg->json_res, _hport ) ? atoi(_hport) : DEFAULT_HTTP_PORT;
			config_xml_remove( xcfg->json_res );
		}
	}
	MSG("HTTP PORT : %d\n", xcfg->http_port);
	make_access_info_file( xcfg->http_port );

	/* TIMEOUT */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/timeout" );
	if( xcfg->json_res ) {
		char _timeout[32];
		xcfg->client_timeout =
			!config_xml_json_value( xcfg->json_res, _timeout ) ? atoi(_timeout) : DEFAULT_CLIENT_TIMEOUT;
		config_xml_remove( xcfg->json_res );
	}
	MSG("CLIENT TIMEOUT : %d\n", xcfg->client_timeout);

	/* MAINIDLE*/
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/mainidle" );
	if( xcfg->json_res ) {
		char _mainidle[32];
		xcfg->main_idle =
			!config_xml_json_value( xcfg->json_res, _mainidle ) ? atoi(_mainidle) : DEFAULT_MAIN_IDLE;
		config_xml_remove( xcfg->json_res );
	}
	MSG("MAIN IDLE TIME : %d seconds\n", xcfg->main_idle);

	/* VERBOSITY */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/verbose" );
	if( xcfg->json_res ) {
		char _loglevel[32];
		sys_loglevel =
			!config_xml_json_value( xcfg->json_res, _loglevel) ? atoi(_loglevel) : LOGLEVEL_DEFAULT;
		config_xml_remove( xcfg->json_res );
	}
	MSG("LOG LEVEL : %d \n", sys_loglevel);

	/* SYNC EXPIRE */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/syncexpire" );
	if( xcfg->json_res ) {
		char _sync[32];
		xcfg->sync_expire =
			!config_xml_json_value( xcfg->json_res, _sync ) ? atoi(_sync) : DEFAULT_SYNC_EXPIRE;
		config_xml_remove( xcfg->json_res );
	}
	MSG("SYNC EXPIRE TIME : %d millisec\n", xcfg->sync_expire);

	/* SHELL PROCESSOR */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/shellproc" );
	if( xcfg->json_res ) {
		char _proc[256];
		if ( !config_xml_json_value( xcfg->json_res, _proc ) ) {
			xcfg->shellproc = (char *)__os_malloc( 256 );
			if (!xcfg->shellproc) {
				ERR("SHELL PROCESSOR NAME(%s) MALLOC ERROR\n", _proc);
				return 1;
			}
			memset( xcfg->shellproc, 0, 256 );
			memcpy( xcfg->shellproc, _proc, strlen(_proc) );
		} else {
			xcfg->shellproc = NULL;
		}
		config_xml_remove( xcfg->json_res );
	}
	MSG("SHELL PROCESSOR : %s\n", xcfg->shellproc);

	/* KEY FILE */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/tls/keyfile" );
	if( xcfg->json_res ) {
		char _kfile[64];
		if ( !config_xml_json_value( xcfg->json_res, _kfile ) ) {
			xcfg->keyfile = (char *)__os_malloc( 64 );
			if (!xcfg->keyfile) {
				ERR("HTTPS::KEY FILE(%s) MALLOC ERROR\n", _kfile);
				return 1;
			}
			memset( xcfg->keyfile, 0, 64 );
			memcpy( xcfg->keyfile, _kfile , strlen(_kfile) );
		} else {
			xcfg->keyfile= NULL;
		}
		config_xml_remove( xcfg->json_res );
	}
	MSG("KEY FILE: %s\n", xcfg->keyfile);

	/* CERT FILE */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/tls/cafile" );
	if( xcfg->json_res ) {
		char _cafile[64];
		if ( !config_xml_json_value( xcfg->json_res, _cafile ) ) {
			xcfg->cafile = (char *)__os_malloc( 64 );
			if (!xcfg->cafile) {
				ERR("HTTPS::KEY FILE(%s) MALLOC ERROR\n", _cafile);
				return 1;
			}
			memset( xcfg->cafile, 0, 64 );
			memcpy( xcfg->cafile, _cafile , strlen(_cafile) );
		} else {
			xcfg->cafile= NULL;
		}
		config_xml_remove( xcfg->json_res );
	}
	MSG("CA FILE: %s\n", xcfg->cafile);

	/* init synchronize */
	pthread_mutex_init( &(xcfg->mut), NULL );
	if( init_timer( &xcfg->timer, (char *)"synctimer", sync_handler, (void *)xcfg ) ) {
		ERR("init_timer::failed\n");
		return 1;
	}

	/* random number seed  - please refer to rand_string function */
	gettimeofday( &xcfg->rand_tv, NULL );
	srand( (xcfg->rand_tv.tv_sec * 1000000) + (xcfg->rand_tv.tv_usec) );

#ifdef _USE_HTTPS_
	/* Create file.... */
	do {
		char cmd[512];

		/* 7300 = 20 years */
		memset(cmd,0,512);
		sprintf(cmd,"openssl req -newkey rsa:2048 -nodes -keyout %s -x509 -days 7300 -out %s -subj %s", 
				xcfg->keyfile, xcfg->cafile, cert_subj);
		system(cmd);
	} while(0);

	/* KEY/CERT PEM */
	do {
		int res,amount;
	        int fd;

		if (xcfg->cafile) {
			fd=open(xcfg->cafile,O_RDONLY);
			if (fd < 0) {
				ERR("%s file error\n",xcfg->cafile);
				break;
			}
			amount = 0;
			memset(cert_pem,0,65536);
			while((res=read(fd,cert_pem+amount,1024))>0)
				amount += res;
			close(fd);
		}
		if (xcfg->keyfile) {
			fd=open(xcfg->keyfile,O_RDONLY);
			if (fd < 0) {
				ERR("%s file error\n",xcfg->keyfile);
				break;
			}
			amount = 0;
			memset(key_pem,0,65536);
			while((res=read(fd,key_pem+amount,1024))>0)
				amount += res;
			close(fd);
		}
	} while(0);
#endif /* _USE_HTTPS_ */

	/*
	 * HTTP server start !!
	 */
	xcfg->httpd =
		MHD_start_daemon(
			MHD_USE_THREAD_PER_CONNECTION |
#ifdef _USE_HTTPS_
			MHD_USE_TLS |
#endif
			MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ITC | MHD_USE_ERROR_LOG,
			xcfg->http_port,
			AcceptCallback,                 NULL,
			DefaultHandlerCallback,         NULL,
			MHD_OPTION_URI_LOG_CALLBACK,    LogCallback, NULL,
			MHD_OPTION_NOTIFY_COMPLETED,    CompletionCallback, NULL,
			MHD_OPTION_NOTIFY_CONNECTION,   ConnectionCallback, NULL,
			MHD_OPTION_CONNECTION_TIMEOUT,  (unsigned int)xcfg->client_timeout,
#ifdef _USE_HTTPS_
			MHD_OPTION_HTTPS_MEM_KEY, key_pem,
			MHD_OPTION_HTTPS_MEM_CERT, cert_pem, 
#endif 
			MHD_OPTION_END );

	/* Syncing into MTD */
	if (merged)
		sync_handler( xcfg );

	/* Synchronize */
	{
		int fd = open(xcfg_sync_tmpfile,O_CREAT|O_RDWR,0666);
		close(fd);
		/* signalling to parent process ... I'm done */
	}

	/* waiting ... */
	xcfg->done = 0;
	while( !xcfg->done )
		sleep( xcfg->main_idle );

	MSG("Shutting down ... \n");

	/* close timer */
	MSG("TIMER DELTE \n");
	pend = pending_timer(&xcfg->timer);
	stop_timer(&xcfg->timer);
	del_timer(&xcfg->timer);

	/* timer module termination */
	timer_module_deinit();

	/* close HTTP server */
	MSG("HTTP SERVICE CLOSE \n");
	MHD_stop_daemon( xcfg->httpd );

	/* storage sync */
	if (pend)
		sync_handler( xcfg );

	/* deleting XML */
	MSG("XML TREE/PARSER REMOVE \n");
	config_xml_exit( xcfg->parser, xcfg->tree );

#ifdef _USE_HTTPS_
	/* Deleting CACERT/KEY files... */
	do {
	        int fd;

		if (xcfg->cafile) {
			fd=open(xcfg->cafile,O_RDONLY);
			if (fd < 0)
				break;
			close(fd);
			unlink(xcfg->cafile);
		}
		if (xcfg->keyfile) {
			fd=open(xcfg->keyfile,O_RDONLY);
			if (fd < 0)
				break;
			close(fd);
			unlink(xcfg->keyfile);
		}
	} while(0);
#endif

	/* delete XCFG main container */
	free(xcfg);

	MSG("SEE YOU~ BYE~ \n");

	return 0;
}

