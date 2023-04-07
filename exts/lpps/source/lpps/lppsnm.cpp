#include <cstdio>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include "transport.h"

int sys_daemon = 0;
int sys_debug = 0;
int use_syslog = 0;

char mpnt[1024]; // mount point 

#ifdef ERR
	#undef ERR
#endif
#ifdef DBG
	#undef DBG
#endif

#define ERR(fmt,args...)	{ \
	if (use_syslog)  \
		syslog(LOG_INFO,"LNM:@:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
	else \
		fprintf(stderr,"LNM:#:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
	}

#define DBG(fmt,args...)	{ \
	if (use_syslog)  \
		syslog(LOG_INFO,"LNM:@:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
	else \
		fprintf(stderr,"LNM:@:%-20s:%-4d: " fmt,(char *)__func__,(int)__LINE__,##args); \
	}

static int start_watcher( int isdir , char * path );
static void scan_folder(const char * folder);

typedef struct {
	pthread_t watcher;
	char * path;
	char * cmd;
	int isdir; /* dir or file */
	int fd;
}__attribute__((aligned)) job_t;

/* Queue structuture.. */
typedef struct {
	#define MAXQ 128
	pthread_mutex_t mut;
	pthread_cond_t  cond;
	int head;
	int tail;

	#define TRANS_NEWDIR  0x0001
	#define TRANS_DELFILE 0x0002
	int  type[ MAXQ ];
	char *trans[ MAXQ ];
}__attribute__((aligned)) trans_queue_t;

static trans_queue_t transq; /* new directory queue */

// special files 
#define MAXSPC 8
static char * spcf[ MAXSPC ] = {NULL,};

// 
// Parameters for reference  (public usage)
//
parameters_t params = {
	.nmid      = DEFAULT_ID,
	.device    = (char *)DEFAULT_DEV,
	.ip        = (char *)strdup( DEFAULT_IP ),
	.port      = DEFAULT_PORT,
	.transport = DEFAULT_TRANSPORT,
	.level     = DEFAULT_LEVEL,
};

static struct option long_options[] = 
	{
		{ "help"   , no_argument      , 0, 'h' },
		{ "no"     , required_argument, 0, 'n' },
		{ "device" , required_argument, 0, 'd' },
		{ "port"   , required_argument, 0, 'p' },
		{ "ip"     , required_argument, 0, 'i' },
		{ "transport",required_argument,0, 't' },
		{ "syslog" , required_argument, 0, 's' },
		{ "level"  , required_argument, 0, 'l' },
		{ "spcfile", required_argument, 0, 'x' },
		{ "folder" , required_argument, 0, 'm' },
		{ NULL     , 0, 0, 0}
	};

static void usage( char *progname )
{
	printf(" %s [options] .. \n", progname);
	printf(" options :    -d/--device <network interface> \n");
	printf("              -n/--no     <ID> \n");
	printf("              -i/--ip     <ip0:ip1>\n");
	printf("              -p/--port   <port> \n");
	printf("              -t/--transport <udp/udp6/ether>\n");
	printf("              -s/--syslog <0/1> \n");
	printf("              -l/--level  <level> \n");
	printf("              -x/--spcfile <file1,file2,..> \n");
	printf("              -m/--mpnt   <mount point> \n");
}

static int special_file( char * path ) 
{
	char *p;
	char *s;
	int cnt;

	if (!path)
		return 0;

	p = strrchr(path,'/');
	if (!p) {
		cnt = 0;
		while ((s = spcf[ cnt++ ])) {
			if (!strncmp(s,path,strlen(s))) 
				return 1;
		}
		return 0;
	}

	cnt = 0;
	while ((s = spcf[ cnt++ ])) {
		if (!strncmp(s,p+1,strlen(s))) 
			return 1;
	}

	return 0;
}

static void create_special_file(char *dname)
{
	char *cmd;
	int fd;

	#define EXTRA 64

	/* Creating special file */
	cmd = (char *)malloc( strlen(dname)+EXTRA );
	if (!cmd) {
		ERR("malloc( strlen(%s)+%d ) - failed\n",dname,EXTRA);
		return ;
	}
	memset(cmd, 0, strlen(dname)+EXTRA );
	snprintf(cmd, strlen(dname)+(EXTRA-1), "%s/%s", dname,spcf[0]);

	if ((fd = open(cmd,O_RDWR)) < 0) {
		/* create a special file */
		memset(cmd, 0, strlen(dname)+EXTRA );
		snprintf(cmd, strlen(dname)+(EXTRA-1), "touch %s/%s", dname,spcf[0]);
		system(cmd);
	} else {
		/* already exists */
		close(fd);
	}

	free(cmd);
}

//
// New Dir Manager 
//
static void * trans_manager( void * queue )
{
	trans_queue_t *tq = (trans_queue_t *)queue;
	int rc;
	int h;
	char *dname;
	job_t *job;

	if (!tq)
		return 0;

	while ( 1 ) {
		rc = pthread_cond_wait( &(tq->cond), &(tq->mut) );
	  	if (rc < 0) {
			ERR("pthread_cond_wait - failed\n");
			continue;
		}

		while ( tq->head != tq->tail ) {
			h = tq->head ;

			switch (tq->type[ h ]) {
			case TRANS_NEWDIR:
				dname = tq->trans[ h ];
				if (dname) {
					dname[ strlen(dname)-1 ] = 0x0; /* eliminating \n */

					DBG("NEWDIR(%s) PROC DONE\n",dname);

					/* if possbile - create a special file */
					create_special_file( dname );
					/* Spawn thread */
					if (!start_watcher( 1 , dname ))
						scan_folder( dname );
					free( dname );
				}
				break;
			case TRANS_DELFILE:
				job = (job_t *)tq->trans[ h ];
				if (job) {
					DBG("DELFILE(%s) PROC DONE\n",job->cmd);

					if (job->fd != -1)
						close(job->fd);

					free(job->cmd);
					free(job->path);
					free(job);
				}
				break;
			default:
				break;
			}

			tq->type[ h ] = -1;
			tq->trans[ h ] = NULL;

			h += 1;
			if (h >= MAXQ)
				h = 0;
			tq->head = h;
		}

		pthread_mutex_unlock( &(tq->mut) );
	}
}

static void trans_request( void *q, int type, char *data )
{
	trans_queue_t *tq = (trans_queue_t *)q;
	int t;

	pthread_mutex_lock( &(tq->mut) );

	t = tq->tail;

	transq.type[ t ] = type;
	transq.trans[ t ] = data;

	t += 1;
	if (t >= MAXQ)
		t = 0;
	tq->tail = t;

	pthread_cond_signal( &(tq->cond) );
	pthread_mutex_unlock( &(tq->mut) );
}

//
// Transport reader task...
//
static void * transport_reader( void * unused )
{
	lpps_pkt_t mesg;
	int cnt;

	while ( 1 ) {
		memset( (char *)&mesg , 0, sizeof(lpps_pkt_t) );
		cnt = transport_recv( params.network , params.fd, &mesg );
		DBG("[RECV] %d bytes - %d %d %x (%s) \n", 
				cnt, mesg.hdr.sid, mesg.hdr.did, mesg.hdr.csum, mesg.data );
	}
}

static void * watch_func( void * param ); // right below...

// run command 
static int start_watcher( int isdir , char * path ) 
{
	int ret;
	job_t *job;
	int len;

	// job list node 
	job = (job_t *)malloc(sizeof(job_t));
	if (!job) {
		ERR("malloc(sizeof(job_t)) failed\n");
		return -1;
	}

	// path ...
	job->path = strdup(path);
	if (!job->path) {
		ERR("strdup(%s) failed\n",path);
		free(job);
		return -1;
	}

	len = strlen(job->path)*2+32; // touch %s/.all ; cat %s/.all ...

	job->cmd = (char *)malloc( len );
	if (!job->cmd) {
		ERR("malloc( %s ) - failed\n",job->path);
		free(job->path);
		free(job);
		return -1;
	}

	memset(job->cmd,0,len);
	memcpy(job->cmd,job->path,strlen(job->path));

	if (isdir) {
		strcat( job->cmd, "/" );
		strcat( job->cmd, spcf[0] );
		strcat( job->cmd, "?deltadir,wait" );
	} else {
		strcat( job->cmd, "?delta,json,wait" );
	}

	job->isdir = isdir;

	// create watcher thread 
	ret = pthread_create( &job->watcher , NULL , watch_func , (void *)job );
	if (ret) {
		ERR("pthread_create() - failed\n");
		free(job->cmd);
		free(job->path);
		free(job);
		return -1;
	}

	// detaching thread - resource maintained in...
	if (pthread_detach( job->watcher ))
		ERR("pthread_detach() - failed\n");

	return isdir ? 0 : -1;
}

// watch function 
static void * watch_func( void * param )
{
	int rc, done;
	job_t *job = (job_t *)param;
	#define LINE 2048
	char *buff;
	fd_set rfds;

	if (!job)
		goto __exit_watch_func;

	// buffer .. 
	buff = (char *)malloc( LINE );
	if (!buff) {
		ERR("malloc( %d )- failed\n",LINE);
		goto __exit_watch_func;
	}

	// transaction open ... 
	job->fd = open(job->cmd,O_RDONLY);
	if (job->fd < 0) {
		ERR("open( %s )- failed\n",job->cmd);
		goto __exit_watch_func;
	}

	DBG("THREAD [ %s %d ] \n", job->cmd, job->fd );

	done = 0;

	while (!done) {
		memset(buff,0,LINE);

		FD_ZERO(&rfds);
		FD_SET(job->fd,&rfds);

		rc = select(job->fd+1,&rfds,NULL,NULL,NULL);
		if (rc <= 0) {
			ERR("FD error %s\n",job->cmd);
			break;
		} 

		// read string...
		rc = read(job->fd, buff, LINE-1);
		if (rc <= 0) {
			ERR("read break %s \n",job->cmd);
			break;
		}

		if (buff[ 0 ] == 0x0) {
			ERR("invalid data %s \n",job->cmd);
			break;
		}

		if (buff[ 0 ] == '@') {
			DBG( "UPD FIL %s/%s \n", job->path, buff);
		} else 
		if (buff[ strlen(buff)-2 ] == '/') {
			/* DIRECTORY CREATION/DELETION */
			switch ( *buff ) { 
			case '+':
				int len;
				char *dname;

				DBG( "NEW DIR %s/%s \n", job->path, buff+2);

				// build up full path 
				len = strlen(job->path)+32+strlen(buff+2);
				dname = (char *)malloc(len);
				if (!dname)
					continue;
				snprintf(dname,len,"%s/%s",job->path,buff+2);

				/*
				 *  THIS IS INSIDE OF CRITICAL SECTION !! 
				 *  NEVER DO THIS HERE !
				 *	(1) CREATE A FOLDER
				 *	(2) CREATE A FILE 
				 *	THESE TWO THINGS SHOULD BE DONE IN DIFFERENT THREAD
				 */

				trans_request( &transq, TRANS_NEWDIR, (char *)dname );
				break;
			case '-':
				// TODO - removing thread ...
				DBG( "DEL DIR %s/%s (%s)\n", 
							job->path, buff+2 /* skipping "-@" */ , job->cmd );
				break;
			default:
				break;
			}
		} else {
			/* FILE CREATION/DELETION */
			switch ( *buff ) {
			case '+':
				int len;
				char *fname;

				DBG( "NEW FIL %s/%s \n", job->path, buff+2 /* skipping "+@" */ );

				// build up full path 
				len = strlen(job->path)+8+strlen(buff+2);
				fname = (char *)malloc(len);
				if (!fname)
					continue;
				snprintf(fname,len,"%s/%s",job->path,buff+2);

				fname[ strlen(fname)-1 ] = 0x0; /* eliminating \n */

				// creating one more...
				start_watcher( 0 , fname );

				free( fname );
				break;
			case '-':
				// TODO - removing thread ...
				DBG( "DEL FIL %s/%s \n", job->path, buff+2 /* skipping "-@" */ );
				break;
			}
		} // if( buff[ strlen(buff) - 1 ] == '/' ) 
	} // while (1) .. 
	DBG("WATCH THREAD %s DONE!!\n", job->cmd);

__exit_watch_func:

	if (buff)
		free(buff);

	//
	// thread is not joined - pthread_exit() will clean up resources. 	
	pthread_exit(0);

	return NULL;
}

static void scan_folder(const char * folder)
{
	DIR *dp;
	struct dirent *entry;
	struct stat st;
	char *fname;
	int len;


	if ((dp = opendir(folder)) == NULL) {
		ERR("%s opendir error\n",folder);
		return ;
	}

	while ( (entry = readdir(dp)) != NULL ) {
		len = strlen(folder)+32+strlen(entry->d_name);
		fname = (char *)malloc(len);
		if (!fname) {
			ERR("malloc( %s + %s + 32 ) - failed\n",folder, entry->d_name);
			break;
		}
		memset(fname, 0, len);
		snprintf(fname,len,"%s/%s",folder,entry->d_name);
		
		stat(fname,&st);
		if (S_ISDIR(st.st_mode)) {
			if (!strcmp(entry->d_name,".") ||
					!strcmp(entry->d_name,"..")) {
				free( fname );
				continue;
			}

			// build up full path 
			DBG( "NEW DIR %s \n", fname);

			create_special_file( fname );

			if (!start_watcher( 1 , fname ))
				scan_folder( fname );

		} else {

			// build up full path 
			if (special_file( entry->d_name )) {
				free( fname );
				continue;
			}

			DBG( "NEW FIL %s|%s \n", folder, entry->d_name);

			start_watcher( 0 , fname );
		}

		free( fname );
	}

	closedir(dp);
}

static int done = 0; // flag

int main(int argc,char *argv[])
{
	int c, option_index;
	int ret;
	pthread_t reader;
	pthread_t dirman;

	memset(mpnt,0,1024); // mount point...

	optind = 0;
	while (1) {
		option_index = 0;
		c = getopt_long(argc,argv, "hn:d:i:s:p:t:l:x:m:", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage( argv[0] );
			exit(0);
		case 'n':
			params.nmid = atoi( optarg );
			break;
		case 'd':
			params.device = strdup( optarg );
			break;
		case 'i':
			params.ip = strdup( optarg );
			break;
		case 'p':
			params.port = atoi( optarg );
			break;
		case 't':
			if (!strcmp(optarg,"udp")) {
				params.transport = TRANS_UDP_IPV4;
			} else
			if (!strcmp(optarg,"udp6")) {
				params.transport = TRANS_UDP_IPV6;
			} else
			if (!strcmp(optarg,"ether")) {
				params.transport = TRANS_IEEE_802_3;
			} else {
				ERR(" Invalid transport - UDP enforced\n");
				params.transport = TRANS_UDP_IPV4;
			}
			break;
		case 'l':
			params.level = atoi( optarg );
			break;
		case 'x':
			{
			char *p, *f;
			int cnt=0;
			p = optarg;
			while (p) {
				f = strchr(p,',');
				if (f)
					*f = 0;
				spcf[cnt++] = strdup(p);
				p = f+1;
				if (cnt == MAXSPC || !f)
					break;
			}
			}
			break;
		case 's':
			use_syslog = atoi( optarg);
			break;
		case 'm':
			strcpy(mpnt,optarg);
			strcat(mpnt,"/");
			break;
		}
	}

	/* ip separation */
	{
		char *p = strchr(params.ip,':');

		*p = 0x0; // null terminating 

		params.ip0 = params.ip;
		params.ip1 = p+1;
	}

	params.fd = 0;

	//
	// create transport 
	params.network = transport_create( params.transport , &params );
	if (!params.network) {
		ERR("transport_create( %d ) failed\n", params.transport);
		return EIO;
	}

	//
	// open transport 
	if ( transport_open( params.network, params.device, &params.fd ) ) {
		ERR("transport_open( .. %s ) - error \n", params.device);
		goto __exit;
	}

	// 
	// create receiver 
	ret = pthread_create( &reader, NULL, transport_reader, NULL );
	if (ret) {
		ERR("pthread_create(,,,() - reader fork error\n");
		goto __exit;
	}

	/* Queue initializer */
	transq.head = 0;
	transq.tail = 0;
	pthread_mutex_init( &transq.mut , NULL );
	pthread_cond_init( &transq.cond , NULL );
	for (int i = 0; i < MAXQ; i++ ) {
		transq.type[ i ]  = -1;
		transq.trans[ i ] = NULL;
	}

	// 
	// create directory handler 
	ret = pthread_create( &dirman, NULL, trans_manager, (void *)&transq );
	if (ret) {
		ERR("pthread_create(,,,() - trans manager fork error\n");
		goto __exit;
	}

	// if not create a special file ... 
	create_special_file( mpnt );

	//prepare for threads to watch over /pps mount 
	if (!start_watcher( 1 , mpnt ))
		scan_folder( mpnt );

	while (!done)
		sleep(1);

__exit:
	if (params.fd) 
		transport_close( params.network , params.fd );

	transport_destroy( params.network );
	return 0;
}

