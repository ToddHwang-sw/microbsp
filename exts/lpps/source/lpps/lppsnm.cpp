#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "transport.h"

int sys_daemon = 0;
int sys_debug = 0;
int use_syslog = 0;

char *mpnt; // mount point 

/*
 *
 */
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

//
//
// base list structure...
typedef struct __list {
        struct __list *next;
        struct __list *prev;
}__attribute__((aligned)) list_t;

//
// bucket_of ..
#define bucket_of(ptr, type, member) ({                              \
                        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
                        (type *)( (char *)__mptr - offsetof(type,member) );})

typedef struct {
	pthread_t watcher;
	char * path;
	char * cmd;
	FILE * fp;
	int isdir; /* dir or file */
	list_t l;
}__attribute__((aligned)) job_list_t;

//
// Job list root 
static list_t * job_root = NULL;
static pthread_mutex_t job_mut = PTHREAD_MUTEX_INITIALIZER;

static int msec_delay = 0 /*300*/; /* processing delay - just for simulating purposes... */

// special files 
#define MAXSPC 8
static char * spcf[ MAXSPC ] = {NULL,};

//
//  L I S T   A P I s
//

// A D D 
static inline void list_add(list_t **r, list_t *item)
{
	list_t *eol;

	pthread_mutex_lock( &job_mut );

	if (!(*r) ) {
		(*r) = item;
		item->next = item;
		item->prev = item;
	} else {
		eol = (*r)->prev;
		(*r)->prev = item;
		item->next = (*r);
		item->prev = eol;
		eol->next = item;
	}

	pthread_mutex_unlock( &job_mut );
}

// D E L 
static inline void list_del(list_t **r, list_t *item)
{
	list_t *p;

	if (!item)
		return ;

	pthread_mutex_lock( &job_mut );

	// chaining 
	p = item->prev;
	p->next = item->next;

	p = item->next;
	p->prev = item->prev;

	if (item == (*r))
		(*r) = item->next; // next ? prev ?

	// orphanage
	item->prev = 
	item->next = NULL;

	// exception case 
	if ( ((*r)->prev == NULL) && ((*r)->next == NULL) )
		(*r) = NULL;

	pthread_mutex_unlock( &job_mut );

}

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
static int run_command( int isdir , char * path ) 
{
	int ret;
	job_list_t *jl;
	int len;

	if (!isdir && special_file( path ))
		return 0;

	// job list node 
	jl = (job_list_t *)malloc(sizeof(job_list_t));
	if (!jl) {
		ERR("malloc(sizeof(job_list_t)) failed\n");
		return -1;
	}

	// path ...
	jl->path = strdup(path);
	if (!jl->path) {
		ERR("strdup(%s) failed\n",path);
		free(jl);
		return -1;
	}

	len = strlen(jl->path)*2 + 32; // touch %s/.all ; cat %s/.all ...

	jl->cmd = (char *)malloc( len );
	if (!jl->cmd) {
		ERR("malloc( %s ) - failed\n",jl->path);
		free(jl->path);
		free(jl);
		return -1;
	}

	memset(jl->cmd,0,len);
	if (isdir) {
		snprintf(jl->cmd, len, "touch %s/.all", jl->path);
		system(jl->cmd);
		memset(jl->cmd,0,len);
		snprintf(jl->cmd,len, "cat %s/.all?deltadir,wait" , jl->path);
	} else {
		snprintf(jl->cmd,len, "cat %s?delta,wait", jl->path );
	}

	jl->isdir = isdir;

	// create watcher thread 
	ret = pthread_create( &jl->watcher , NULL , watch_func , (void *)jl );
	if (ret) {
		ERR("pthread_create() - failed\n");
		free(jl->cmd);
		free(jl->path);
		free(jl);
		return -1;
	}
	if (pthread_detach( jl->watcher ))
		ERR("pthread_detach() - failed\n");

	list_add( &job_root, &jl->l ); // add to list 

	// success !!
	return 0;
}

static void setup_watcher(const char * folder);

// watch function 
static void * watch_func( void * param )
{
	job_list_t *job = (job_list_t *)param;
	#define LINE 1024
	static char buff[LINE];

	if (!job)
		goto __exit_watch_func;

	//printf( "[ %08x ] cmd=%s !!\n",(int)job->watcher,job->cmd);

	// watch ... 
	job->fp = popen(job->cmd,"r");
	if (!job->fp) {
		ERR("popen( %s )- failed\n",job->cmd);
		goto __exit_watch_func;
	}

	while (1) {
		memset(buff,0,LINE);
		if (!fgets(buff,LINE,job->fp)) { // getting a result !!
			//
			// When LPPS_HIST_NULL message arrives, 
			// the control will fall into here...
			//
			break;
		}

		// null terminating....
		buff[ strlen(buff)-1 ] = 0x0;

		//printf( "[ %08x ] INPUT %s !!\n",(int)job->watcher,buff);

		if (buff[ 0 ] == '@' || buff[0] == 0x0) {
			memset(buff,0,LINE);
			if (!fgets(buff,LINE,job->fp))  // getting a result !!
				break;

			printf( "[ %08x ] UPD FIL %s|%s \n", (int)job->watcher, job->path, buff);
			if (msec_delay)
				usleep(msec_delay*1000);
		} else 
		if (buff[ strlen(buff)-1 ] == '/') {
			/* DIRECTORY CREATION/DELETION */

			if (*buff == '+') {
				int len;
				char *dname;

				printf( "[ %08x ] NEW DIR %s|%s \n", 
						(int)job->watcher, job->path, buff+2 /* skipping "+@" */ );
				if (msec_delay)
					usleep(msec_delay*1000);

				// build up full path 
				len = strlen(job->path)+8+strlen(buff+2);
				dname = (char *)malloc(len);
				if (!dname)
					continue;
				snprintf(dname,len,"%s/%s",job->path,buff+2);

				// creating one more...
				run_command( 1 , dname );

				// WHY ???? Hmm...
				// scan below ...
				setup_watcher( dname );

				free( dname );
			} else 
			if (*buff == '-') {
				// TODO - removing thread ...
				printf( "[ %08x ] DEL DIR %s|%s \n", 
						(int)job->watcher, job->path, buff+2 /* skipping "-@" */ );
				if (msec_delay)
					usleep(msec_delay*1000);
			} else {
			}
		} else {
			/* FILE CREATION/DELETION */

			if (*buff == '+') {
				int len;
				char *fname;

				printf( "[ %08x ] NEW FIL %s|%s \n", 
						(int)job->watcher, job->path, buff+2 /* skipping "+@" */ );
				if (msec_delay)
					usleep(msec_delay*1000);

				// build up full path 
				len = strlen(job->path)+8+strlen(buff+2);
				fname = (char *)malloc(len);
				if (!fname)
					continue;
				snprintf(fname,len,"%s/%s",job->path,buff+2);

				// creating one more...
				run_command( 0 , fname );

				free( fname );
			} else 
			if (*buff == '-') {
				// TODO - removing thread ...
				printf( "[ %08x ] DEL FIL %s|%s \n", 
						(int)job->watcher, job->path, buff+2 /* skipping "-@" */ );
				if (msec_delay)
					usleep(msec_delay*1000);
			} else {
			}
		} // if( buff[ strlen(buff) - 1 ] == '/' ) 

		fflush( job->fp );
	}

__exit_watch_func:
	list_del( &job_root, &job->l ); // delete from list 

	fclose(job->fp);

	free(job->cmd);
	free(job->path);
	free(job);

	//
	// thread is not joined - pthread_exit() will clean up resources. 	
	pthread_exit(0);

	return NULL;
}

static void setup_watcher(const char * folder)
{
	DIR *dp;
	struct dirent *entry;
	struct stat st;
	char *fname;
	int len;

	if ((dp = opendir(folder)) == NULL) {
		ERR("%s open error\n",folder);
		return ;
	}

	if (chdir(folder)) {
		closedir(dp);
		ERR("chdir( %s ) failed\n",folder);
		return ;
	}

	while ( (entry = readdir(dp)) != NULL ) {
		lstat(entry->d_name,&st);
		if (S_ISDIR(st.st_mode)) {
			if (!strcmp(entry->d_name,".") ||
					!strcmp(entry->d_name,".."))
				continue;

			// build up full path 
			len = strlen(folder)+8+strlen(entry->d_name);
			fname = (char *)malloc(len);
			if (!fname)
				goto __exit_setup_watcher;

			snprintf(fname,len,"%s/%s",folder,entry->d_name);

			printf( "[ %08x ] NEW DIR %s|%s \n", (int)0, folder, entry->d_name );

			run_command( 1 , fname );

			setup_watcher( fname );

			free( fname );
		} else {
			// build up full path 
	
			if (special_file( entry->d_name ))
				continue;

			len = strlen(folder)+8+strlen(entry->d_name);
			fname = (char *)malloc(len);
			if (!fname)
				goto __exit_setup_watcher;
			snprintf(fname,len,"%s/%s",folder,entry->d_name);

			printf( "[ %08x ] NEW FIL %s|%s \n", (int)0, folder, entry->d_name);

			run_command( 0 , fname );

			free( fname );
		}
	}
  __exit_setup_watcher:
	if (chdir(".."))
		ERR("chdir(..) failed\n");
	closedir(dp);
}


static int done = 0; // flag

int main(int argc,char *argv[])
{
	int c, option_index;
	int ret;
	pthread_t reader;
	/* lpps_pkt_t msg; */

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
			mpnt = strdup( optarg );
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

	//prepare for threads to watch over /pps mount 
	run_command( 1 , mpnt );
	setup_watcher( mpnt );

	while (!done)
		sleep(1);

__exit:
	if (params.fd) 
		transport_close( params.network , params.fd );

	transport_destroy( params.network );
	return 0;
}

