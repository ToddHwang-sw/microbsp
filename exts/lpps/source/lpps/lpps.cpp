#include <fuse.h>
#include <fuse_opt.h>
#include <fuse_i.h>
#include <fuse_lowlevel.h>
#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include "malloc.h"
#include "os.h"
#include "sig.h"
#include "plist.h"
#include "fhandle.h"
#include "conf.h"

#include "transport.h"

using namespace std;

#define KILO(n)	((n) << 10)
#define MEGA(n)	(KILO(n) << 10)

//
// Debugging mode
// 	0 - No debug
// 	1 - lpps only debugging
// 	2 - libfuse level debugging
// 	3 - Memory debugging 
//
int sys_debug = DEBUG_NONE;

//
// lpps running mode - daemon/foreground??
//
int sys_daemon;

//
// syslog logigging 
//
int use_syslog = 0;

//
// Memory block...
//
int sys_rmem = MEGA(64);
struct memspace *os_memspc = NULL;
pthread_mutex_t os_memlock;

#define TMP_MNTPOINT	"/mnt/lpps"

//
// It is created by libfuse, but we don't need this. 
//
#define INVALID_FILE	".Trash"

// defaut fuse arguments - should be used
//
// default_permissions + allow_other
//
#define DEFAULT_ARGS	"default_permissions,allow_other"

#define BLKSIZE		4096

//
// ASSERT upon path 
//
#define PATH_CHECK(p)	{ if (!p) { DBG(" NULL PATH\n"); return 0; } }

// object type ...
enum {
	LPPS_FILE = 0x1,
	LPPS_DIR,
	LPPS_LINK,
	LPPS_TYPE
};

typedef struct fdobj {
	char *path;	// file path 
	char *sym;	// symbolic link 
	int type; 	// LPPS_FILE/LPPS_DIR 
	int uid;	// user id 
	int gid;	// group id 
	int mode;
	int size;
	int blksize;	// block size 
	int blocks;	// how many blocks 
	int nlink;
	time_t atime;
	time_t ctime;
	time_t mtime;
	char * fline;	// first line  - @<filename>
	struct fdobj * next;
	ppsl *tuples;
	ppsl *delta;
	ppriv ** rwtasks; // outstanding file access 
}__attribute__((aligned)) lpps_inode_t;
#define FN_SIZE(p)	(((p)->fline)?strlen((p)->fline):0)
#define FILE_SIZE(p,m)	(((p)->size * (((m) == 0)?1:(m))) + FN_SIZE(p))

typedef struct {
	struct fuse_operations *oper;
	struct fuse * fuse;
	struct fuse_session *session;
	lpps_inode_t *root;	// file/directory : actual file 
	char *mntpt;		// mount point 
	os_rwlock_t *mut;	// access lock 
	os_sig_t *hist_lock;	// history lock 
	conf_t *conf;		// configuration node 
	list_t *hroot;	// history transaction root 
	parameters_t net;
	pthread_t netreader;    // multicast message reader 
}__attribute__((aligned)) lpps_t;
static lpps_t lpps = {0,};

//
// callback level lock / unlock ...
//
#define LPPS_UNLOCK()	os_rwunlock(lpps.mut)
#define LPPS_RLOCK()	os_rlock(lpps.mut)
#define LPPS_WLOCK()	os_wlock(lpps.mut)

static struct fuse_operations lpps_oper = {NULL,};

//
// These functions are referenced from C file ...
//
extern "C" {
	void lpps_ll_interrupt( pthread_t id );
};

//
// Options processing block 
//
static struct options {
	const char *user; // username should be in '/etc/passwd'
	const char *conf; // configuration file name 
	int daemon;	  // daemonize option 
	int syslog;	  // syslog 
	int memory;	  // memory space
	int bmemory;	  // memory bottom line 
	int json_buflen;  // json buffer length   
	int debug;	  // debug mode 
	int uid;
	int gid;
	int esize;	  // size expand for "?json" option  
	int mshare;	  // max number of outstanding R/W instances at the same time 
	int background;	  // Fuse background
	int mrtimeout;	  // Multiple Reader Timeout 
	int maxtask;	  // Max reader/writer per a file
	int history;	  // History size
	char *fn_memtrace;  // filename for memory trace 
	char *fn_acctrace;  // filename for access trace for directory
	int network;      // multicast broadcast 
	char *ip;         // multicast ip
	int port;         // multicast udp port
	enum transport_type transport;  // udpv4/udpv6/ethernet
	int netid;        // network id
	char *netdev;     // network interface 
} options;
#define OPTION(t, p) { t, offsetof(struct options, p), 1 }

static const struct fuse_opt option_spec[] = {
         OPTION("--user=%s",   user),
         OPTION("--conf=%s",   conf),
         OPTION("--daemon=%d", daemon),
         OPTION("--syslog=%d", syslog),
         FUSE_OPT_END
};

//
// allocates history block 
//
static __inline__ lpps_hist_blk_t * make_hist_block( char *path, int mode )
{
	lpps_hist_blk_t * p;

	p = (lpps_hist_blk_t *)os_malloc( sizeof(lpps_hist_blk_t) ) ;
	if (!p) {
		ERR(" os_malloc( sizeof(lpps_hist_blk_t) ) for %s FAILED\n",path);
		return NULL;
	}

	// make a new one 
	p->mode = mode;
	p->path = path ? os_strdup(path) : NULL ;  // path , NULL --> termination indication 
	if (path && (!p->path)) {
		os_free(p);
		ERR(" os_strdup(%s) failed \n", path );
		return NULL;
	}

	return p;
}

static __inline__ int __history_lock( char *f )
{
	int err = os_sig_trylock( lpps.hist_lock );

	if (err) 
		ERR(" %s : HISTORY LOCK FAILED\n", f);

	return err;
}

#define history_lock()	__history_lock( (char *)__func__ )

static __inline__ void history_unlock( int lockerr )
{
	if (!lockerr)
		os_sig_unlock( lpps.hist_lock );
}

//
// removes history block 
//
static __inline__ void remove_hist_block( lpps_hist_blk_t *p )
{
	if (!p)
		return ;

	if (p->path) {
		DBG(" REMOVE HIST (%d %s) \n",p->mode, p->path);
		os_free(p->path);
	} else
		DBG(" REMOVE HIST (%d NULL) \n",p->mode);

	os_free(p);
}

static void lpps_hist_ack( ppriv * item);

//
// parent_of ..
#define parent_of(ptr, type, member) ({                              \
                        const typeof( ((type *)0)->member ) *__mptr = (ptr); \
                        (type *)( (char *)__mptr - offsetof(type,member) );})

static __inline__ int hist_queue_make( lpps_hist_queue_t *q , int size )
{
	if (!q)
		return -1;

	q->blks = (lpps_hist_blk_t **)os_malloc( size * sizeof(lpps_hist_blk_t *) );
	if (!q->blks) {
		DBG("os_malloc( size=%d x sizeof(lpps_hist_blk_t) )::failed\n",size);
		q->blks = NULL;
		q->size = 0;
		q->head = q->tail = 0;
		return -1;
	}

	q->size = size;
	q->head = 
	q->tail = 0;

	return 0;
}

// adding an item into a queue .
static __inline__ int hist_queue_put( lpps_hist_queue_t * q, lpps_hist_blk_t *p )
{
	int ntail;

	if (!q || !p)
		return -1;

	if (!q->size) // not inited 
		return -1;

	// update next tail position 
	ntail = (q->tail + 1);
	if (ntail == q->size)
		ntail = 0;

	if (ntail == q->head)
		return -1;

	// insert a data in a queue 
	q->blks[ q->tail ] = p;

	{
	ppriv * parent = parent_of( q, ppriv, hq );
	DBG(" path=%s[%d] -> %s\n",p->path?p->path:"EXIT",p->mode,parent->path);
	}

	// update tail pointer 
	q->tail = ntail;

	return 0;
}

// getting an item from a queue 
static __inline__ lpps_hist_blk_t * hist_queue_get( lpps_hist_queue_t * q )
{
	int nhead; 
	lpps_hist_blk_t * p;

	if (!q)
		return NULL;

	if (!q->size) // not inited 
		return NULL;

	if (q->tail == q->head)
		return NULL;

	// update next head position 
	nhead = (q->head + 1);
	if (nhead == q->size)
		nhead = 0;

	// insert a data in a queue 
	p = q->blks[ q->head ];

	DBG(" path=%s[%d] [ %d %d ] \n",p->path?p->path:"EXIT",p->mode,(int)q->head,(int)q->tail);

	// update tail pointer 
	q->head = nhead;

	return p;
}

static __inline__ int hist_queue_delete( lpps_hist_queue_t * q )
{
	int cnt = 0;

	if (!q->size) // not inited 
		return 0 ;

	while ( q->tail != q->head ) {
		++cnt;
		remove_hist_block( hist_queue_get( q ) );
	}

	os_free( q->blks );

	q->blks = NULL;

	return cnt;
}


//
//  L I S T   A P I s
//
static __inline__ void list_init( list_t *item )
{
	if (!item)
		return ;

	item->next =
	item->prev = NULL;

	return ;
}

// A D D 
static __inline__ void __list_add(list_t **r, list_t *item)
{
	list_t *eol;

	/* check orphanage */
	if ((item->prev != NULL) || 
			(item->next != NULL) ) {
		ERR(" NOT ORPHAN NODE !!\n");
	}

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
}

static void list_add( list_t *item )
{
	__list_add( &(lpps.hroot) , item );
}

// D E L 
static __inline__ void __list_del(list_t **r, list_t *item)
{
	list_t *p;

	if (!item)
		return ;

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
}

static void list_del( list_t *item )
{
	__list_del( &(lpps.hroot), item );
}

static int priv_hist_enabled( ppriv * priv )
{
	if (!priv)
		return 0;

	/* refer to list_init */
	return ((priv->l.next != NULL) && (priv->l.prev != NULL)) ? 1 : 0;
}

//
// Looking for specific history private structure...
//
static ppriv * priv_hist_search( char *path )
{
	list_t *trace;
	ppriv *priv;

	if (!lpps.hroot)
		return NULL;

	trace = lpps.hroot;

	do {
		priv = parent_of( trace, ppriv, l );

		// sanity check 
		if (!priv)
			goto __next_one;

		if (!priv->path)
			goto __next_one;

		if (!strcmp(path,priv->path))
			return priv;
__next_one:
		trace = trace->next;
	} while ( lpps.hroot != trace );

	return NULL;
}

static int priv_hist_setup( ppriv * priv , char * path )
{
	int locked;

	if (!priv)
		return -1;

	locked = history_lock();

	if (priv_hist_enabled(priv)) {
		history_unlock( locked );
		return -1;
	}

	if (priv->path) 
		ERR(" priv->path=(%s) already defined !!\n",priv->path);

	priv->path   = os_strdup(path);  // path ...
	if (!priv->path) {
		ERR(" os_strdup(%s)::failed\n",path);
		history_unlock( locked );
		return -1;
	}
	if ( hist_queue_make( &(priv->hq) , 8 ) ) {
		ERR("HIST QUEUE (%s) MAKE FAILiED \n",priv->path); 
		history_unlock( locked );
		return -1;
	}

	list_add( &(priv->l) );

	DBG(" %s \n",path);

	history_unlock( locked );
	return 0;
}

//static int priv_hist_delete( ppriv *priv , int is_interrupt )
static int priv_hist_delete( ppriv *priv )
{
	int locked;
	//int todelete;

	if (!priv)
		return -1;

	locked = history_lock();

	if (!priv_hist_enabled(priv)) {
		if (priv->path)
			ERR(" %s priv is not enabled\n",priv->path);
		history_unlock( locked );
		return -1;
	}

	if (priv->path) {
		DBG(" PATH %s \n",priv->path);
		os_free(priv->path);
	} else
		DBG(" PATH NONE \n");

	priv->path = NULL;

	if (priv->hq.blks) {
		int lost;
		if ( (lost = hist_queue_delete( &priv->hq )) > 1 )
			ERR(" LOST MESSAGES %d \n", lost);
	}
	priv->hq.blks = NULL;
	priv->hq.size = 0;

	list_del( &(priv->l) );

	history_unlock( locked );
	return 0;
}

// check currently any of instance to wait for the transaction of file...
static __inline__ int file_onwait( lpps_inode_t * fl )
{
	int i;
	ppriv * priv;

	if (!fl)
		return 0;

	for (i = 0; i < options.maxtask; i++) {
		priv = fl->rwtasks[ i ];
		//
		// file is opened or file is under access....
		//
		if (priv)
			return 1;
	}

	return 0;
}

static int wakeup_hist_task( ppriv *item , lpps_hist_blk_t * p )
{
	int ret = 0;
	int skip_lock = 0;

	if (!item)
		return -1;

	//
	// UNRESOLVED BUG ??
	// FAILED TO FIGURE OUT WHY IT WORKS LIKE THIS..
	//
	// #1. The following "lock" looks to be used but it causes 100% deadlock situation. 
	//     LPPS stops working and hangs if it would be used. 
	// #2. Below os_sig_wakeup() calls os_sig_unlock( item->d.sig ). 
	//
	if (!skip_lock) {
		DBG(" WAKE - LOCK \n");
		os_sig_lock( item->sig ); 
	}

	if ( hist_queue_put( &(item->hq) , p ) )
		ERR(" HIST PUT [ %s ]ERR \n", p->path );

	os_sig_wakeup( item->sig , skip_lock );

	DBG(" WAKE & ACK WAIT %s \n",item->path );

	return ret;
}

//
//  "/a/b/c/d/hello" 
//  "/a/b/c/d/e/jello"  --- They are not the same path 
//
static __inline__ int samepath( char *path1 , char *path2 )
{
	char * pPath1 = strrchr(path1,'/');
	char * pNew   = strrchr(path2,'/');

	if ( (size_t)(pPath1 - path1) != (size_t)(pNew - path2) )
		return 0;

	if ( strncmp(path1,path2,(size_t)(pPath1 - path1)) )
		return 0;

	DBG(" SAME PATH [%s|%s] --> OK \n",path1,path2);

	return 1;
}

//
// print format for ".all" file. 
//
static __inline__ const char * hist_print_fmt(int mode)
{
	const char *fmt;

	switch (mode) {
	case LPPS_HIST_MKFILE: fmt = (const char *)"+@%s\n";  break;
	case LPPS_HIST_RMFILE: fmt = (const char *)"-@%s\n";  break;
	case LPPS_HIST_MKDIR:  fmt = (const char *)"+@%s/\n"; break;
	case LPPS_HIST_RMDIR:  fmt = (const char *)"-@%s/\n"; break;
	default:  fmt = (const char *)"?@s?\n"; break;
	}

	return fmt;
}

//
// Used as an argument for "reason" paramater of lpps_stop_task 
//
#define STOP_INTR 0x1000

//
// Stop all "?wait" task for a specific file...
//
static __inline__ int lpps_stop_task( lpps_inode_t *node, int id , int reason ) 
{
	int i, ret;
	ppriv *priv;
	int signalled = 0;

	if (!node)
		return 0;

	for (i = 0; i < options.maxtask; i++) {
		priv = node->rwtasks[ i ];
		if (!priv)
			continue;
		//
		// signal_waiting does not need to be checked. 
		// This just can run acquiring WRITE lock - All the lpps_read() task 
		// will be out of this control. 
		//
		if (id) {
			if (priv->unique != id)
				continue;
		}

		switch( priv->mode ) {

		case READ_WORK:
			//
			// terminates read job ...
			//
			// Bug Fix. - os_sig_lock( priv->sig ) function 
			// may result in infinite deadlock here !! 
			//
			// T1:  "if ( !os_sig_wait( priv->sig , 0 ) ) { " at lpps_read:#1806 - LOCK OBTAINED 
			// T2:  CTRL-C interrupt arrives and LPPS_WLOCK - DONE 
			// T3:  os_sig_lock( priv->sig ) will not be supposed to get obtain lock due to "T1"
			// T4:  lpps_read() cannot resolve this since it cannot proceed over #1814 due to "T2"
			// .... DEADLOCK 
			//
			// Replacing os_sig_lock by os_sig_trylock !! 
			//
			ret = os_sig_trylock( priv->sig );
			bit_adds(priv->sig->state,OS_LOCK_STATE_DONE);

			//
			// called by interrupt CTRL-C
			//
			if (reason == STOP_INTR)
				bit_adds(priv->sig->state,OS_LOCK_STATE_INTR);

			DBG(" %s [%d] INTERRUPT WAKE \n",node->path, priv->fh);
			os_sig_wakeup( priv->sig , ret );
			++signalled; // signalling...
			DBG(" %s INTERRUPT WAKE DONE \n",node->path);
			break;

		case WRITE_WORK:
			//
			// TODO - Something ??
			//
			DBG(" %s [%d] WRITE JOB RELEASE \n",node->path, priv->fh);
			if (priv->ackevt)
				os_event_send( priv->ackevt , ACK_RELEASE | ACK_SHUTDOWN ); 
			break;
		}

		//
		// if id is specific, only correspoding task will be affected...
		//
		if (id)
			break;
	}

	//
	// Waiting for this...
	//
	if (signalled) {
		DBG(" %s - DONE <%d> \n",node->path,i);
		return 1;
	} 

	return 0;
}

//
// invoked from libfuse layer. 
//
void lpps_ll_interrupt( pthread_t id )
{
	lpps_inode_t *trace;

	//
	// lpps_write() function would try to acquire WLOCK(). 
	// This function shall not be overlapped with lpps_write(). 
	//
	LPPS_WLOCK();

	trace = lpps.root;
	while (trace) {
		if(lpps_stop_task( trace , id , STOP_INTR ))
			break;
		trace = trace->next;
	}

	LPPS_UNLOCK();
}

// signal handler can be installed here...
// Please look into "EXTRA_WORKAROUND" ...
static int lpps_signal_handler(struct fuse_sesson *session, int signo)
{
	lpps_inode_t *trace;
	int i;
	ppriv *priv;
	int ret = 0;

	LPPS_WLOCK();

	//
	// signalling 
	//
	trace = lpps.root;
	while (trace) {
		for (i = 0; i < options.maxtask; i++) {
			priv = trace->rwtasks[ i ];
			if (priv) {
				//
				// Any of read/write transaction...
				//
				ret = -1;
				goto finish_SIGNAL_HANDLER;
			}
		}
		trace = trace->next;
	}

	// clean up all of .all?deltadir,wait transactions 
	//lpps_stop_hist_task( 0 );

	if (options.network) {
		pthread_cancel( lpps.netreader ); // work or not ??? - not guaranteed 
		if (lpps.net.fd) 
			transport_close( lpps.net.network , lpps.net.fd );
		usleep( 300*1000 ); // 300 misec ...
		transport_destroy( lpps.net.network );
	}

	//
	// Only none of blocked read with "?wait" option and none of writing transaction exist.
	// We can shutdown this file system.
	//
	DBG(" successfully done\n");

finish_SIGNAL_HANDLER:

	LPPS_UNLOCK();
	return ret;
}

static int lpps_special_paths( const char *path );

/* - used only lpps_unlink */
/* This is added in 07/29/2021 */
static void __inline__ cleanup_priv_tasks(lpps_inode_t *fl) 
{
	int index;
	ppriv *priv;

	DBG(" ENTERED !!! ( %p ) \n",fl);

	for (index = 0; index < options.maxtask; index++) {
		priv = fl->rwtasks[ index ];
		if (!priv)
			continue;
		fl->rwtasks[ index ] = NULL;

		DBG(" CLEAN UP PATH %s\n",priv->path?priv->path:"NONE");

		put_fh( priv->fh );
		os_sig_delete( priv->sig ); 
		if (priv->linebuf)
			os_free(priv->linebuf);
		if (priv->ackevt) {
			os_event_send( priv->ackevt , ACK_RELEASE ); 
			DBG(" ACKEVT %s\n",priv->path?priv->path:"NONE");
		}

		if (lpps_special_paths(priv->path) == 0) {
			//
			// 08/17 - For normal file, the part of priv structure is
			// deleted in here... priv structure of the special file 
			// will be deleted in lpps_read(). 
			//
			if (priv->path)
				DBG(" NORMAL FILE HAS PATH ??? %s \n", priv->path);

			DBG(" NORMAL FILE DELETE %s \n", priv->path);

			os_free( priv ); // normal file 
		} else {
			DBG(" SKIPPING DELETE %s \n", priv->path);
		}
	}
	DBG(" EXIT !!! \n");
}

/* Type of files.. */
enum {
	MEMTRACE_FILE = 10,
	ACCTRACE_FILE ,
	LASTTYPE_FILE
};

static int lpps_special_paths( const char *path )
{
	char *absp;

	if (!path)
		return 0;

	absp = strrchr((char *)path,'/');

	if ( !strncmp(absp+1,
			options.fn_memtrace,
			strlen(options.fn_memtrace)) ) 
		return MEMTRACE_FILE;

	if ( !strncmp(absp+1,
			options.fn_acctrace,
			strlen(options.fn_acctrace)) ) 
		return ACCTRACE_FILE;

	// not specific file 
	return 0;
}

//
// Memory dumper 
//
static __inline__ int lpps_dumpmem( char *buf, int bufsz, char * p, int size )
{
	int i,k;
	int e;
	int l = 0;

	/* sanity check */
	if (!p || (size <= 0))
		return 0;

#define _decode_char(x)	\
		( (isdigit((x)) || isalpha((x)) || ( \
			  ((x) == '!') || ((x) == '@') || ((x) == '#') || ((x) == '$') || ((x) == '%') || ((x) == '^')  ||  \
			  ((x) == '&') || ((x) == '*') || ((x) == '(') || ((x) == ')') || ((x) == '+') || ((x) == '-')  ||  \
			  ((x) == '_') || ((x) == '=') || ((x) == '{') || ((x) == '}') || ((x) == '[') || ((x) == ']')  ||  \
			  ((x) == ',') || ((x) == '.') || ((x) == ';') || ((x) == ':') || ((x) == '"') || ((x) == '\'') ||  \
			  ((x) == '/') || ((x) == '/') || ((x) == '~') || ((x) == '<') || ((x) == '>') )) ? \
			(x) : 0x20 )

	//ERR(" %p %d [ %p %d ] \n", buf, bufsz, p, size);

	i = 0;
	l = 0;
	while( i < size ) {
		// printout...
		for ( k = i; (k < i + 32) && (k < size); k++ ) {
			snprintf(buf+l,bufsz-l,"%02x ", (p[ k ]& 0xFF));
			l = strlen(buf);
			if ((bufsz-l) <= 1)
				goto __exit_dump;
		}
		e = k;

		// trailing....
		for ( ; (k < i + 32); k++ ) {
			snprintf(buf+l,bufsz-l,"   ");
			l = strlen(buf);
			if ((bufsz-l) <= 1)
				goto __exit_dump;
		}

		snprintf(buf+l,bufsz-l,"| ");
		l = strlen(buf);
		if ((bufsz-l) <= 1)
			goto __exit_dump;

		// character 
		for ( k = i; k < e; k++ ) {
			snprintf(buf+l,bufsz-l,"%c", _decode_char( p[ k ] ));
			l = strlen(buf);
			if ((bufsz-l) <= 1)
				goto __exit_dump;
		}

		snprintf(buf+l,bufsz-l,"\n");
		l = strlen(buf);
		if ((bufsz-l) <= 1)
			goto __exit_dump;
	
		i = k; /* update main index */
	}

__exit_dump:
	//ERR(" SIZE = %d %d \n",l,strlen(buf));
	return l;
}

// multicast message receiver ... 
static void * lpps_transport_reader( void * unused )
{
	lpps_pkt_t mesg;
	int cnt;

	while ( 1 ) {
		memset( (char *)&mesg , 0, sizeof(lpps_pkt_t) );
		cnt = transport_recv( lpps.net.network , lpps.net.fd, &mesg );
		DBG(" NET::[RECV] %d bytes - %d %d %x \n", cnt, mesg.hdr.sid, mesg.hdr.did, mesg.hdr.csum );
		if (cnt <= 0)
			break;
	}

	DBG(" NET: RECV FINISHED\n");

	pthread_exit(0);
}

static int lpps_multicast_setup( void )
{
	int ret;
	char *p = strchr(options.ip,':'); // IP separation 

	*p = 0x0; // null terminating 

	lpps.net.ip0 = options.ip;  // source ip
	lpps.net.ip1 = p+1;         // destination ip

	lpps.net.device = options.netdev;
	lpps.net.nmid   = options.netid;
	lpps.net.port   = options.port;

	lpps.net.fd = 0;  // network socket 

	//
	// create transport 
	lpps.net.network = transport_create( options.transport , &lpps.net );
	if (!lpps.net.network) {
		ERR(" NET::transport_create( %d ) failed\n", options.transport );
		return -1;
	}

	//
	// open transport 
	if ( transport_open( lpps.net.network, options.netdev, &lpps.net.fd ) ) {
		ERR(" NET::transport_open( .. %s ) - error \n", options.netdev );
		goto __multicast_setup_exit;
	}

	// 
	// create receiver 
	ret = pthread_create( &lpps.netreader, NULL, lpps_transport_reader, NULL );
	if (ret) {
		ERR(" NET::pthread_create(,,,() - reader fork error\n");
		goto __multicast_setup_exit;
	}

	return 0;

__multicast_setup_exit:
	if (lpps.net.fd) 
		transport_close( lpps.net.network , lpps.net.fd );

	transport_destroy( lpps.net.network );
	return -1;
}

static int lpps_net_multicast( uint8_t id, enum transport_event type, const char * msg )
{
	lpps_pkt_t pkt;
	int len = 0;
	char * data;

	if (!options.network)
		return -1;

	if (!msg)
		return -1;

	len = strlen( msg );

	// check payload size 
	if ((len + 4) >= (PAYLOAD_LENGTH - 4))
		len = (PAYLOAD_LENGTH - 8);  // adjusted 

	// 4 bytes alignment 
	len &= ~0x3;
	len += 4;

	pkt.hdr.len  = htonl( len );
	pkt.hdr.sid  = id;
	pkt.hdr.type = type;

	data = (char *)&(pkt.data);

	memset( data, 0, len );     // zeroing 
	strncpy( data, msg, len );  // payload copy 

	return transport_send( lpps.net.network , lpps.net.fd , type , &pkt );
}

//
//
//
// T O O L K I T     F U N C T I O N S
//
//
//
//
#if 0 
static void lpps_list_print( lpps_inode_t **root )
{
	lpps_inode_t *trace = (*root);

	printf("\n[");
	while (trace) {
		printf(" %s ", trace->path);
		trace = trace->next;
	}
	printf("]\n");
}

static __inline__ char * build_acctrace_filename( char * path )
{
	int  n;
	char *fn;

	if (!path)
		return NULL;

	n = strlen(path) + strlen(options.fn_acctrace) + 4;  // length 
	fn = (char *)os_malloc( n ); 
	if (!fn) 
		return NULL;

	memset(fn, 0, n);
	snprintf(fn,n,"%s/%s",path,options.fn_acctrace);  

	return fn;
}
#endif 

static void lpps_list_push( lpps_inode_t **root, lpps_inode_t *elem )
{
	if (!elem)
		return ;

	if ( (*root) == NULL )
		(*root) = elem;
	else {
		elem->next = (*root);
		(*root) = elem;
	}
}

static lpps_inode_t * lpps_list_pull( lpps_inode_t **root, char *path )
{
	lpps_inode_t *prev;
	lpps_inode_t *trace = (*root);

	prev = trace;
	while (trace) {
		if ( !strcmp( trace->path , path ) ) {
			if (prev == trace) {
				(*root) = trace->next;
			} else {
				prev->next = trace->next;
			}
			trace->next = NULL; // orphanage
			return trace;
		}
		prev = trace;
		trace = trace->next;
	}

	return NULL;
}

// only used for read process .. 
static lpps_inode_t * lpps_list_peek( lpps_inode_t **root, char *path )
{
	lpps_inode_t *trace = (*root);

	while (trace) {
		if ( !strcmp( trace->path , path ) )
			return trace; 
		trace = trace->next;
	}

	return NULL;
}

static void lpps_node_clean( lpps_inode_t *obj )
{
	if (!obj) {
		ERR(" NULL PARAM\n");
		return;
	}

	// signalling all tasks killed !! 
	lpps_stop_task( obj, 0, 0 ); 

	if (obj->path)
		os_free(obj->path);
	if (obj->sym)
		os_free(obj->sym);
	if (obj->fline)
		os_free(obj->fline);
	if (obj->tuples)
		lpps_ppsl_remove( &(obj->tuples) );
	if (obj->delta)
		lpps_ppsl_remove( &(obj->delta) );
	if (obj->rwtasks) {
		os_free(obj->rwtasks);
		obj->rwtasks = NULL;  /* ??????? */
	}

	os_free(obj);
}

static int lpps_res_size( lpps_inode_t *obj )
{
	if (!obj)
		return 0;

	return obj->blocks * obj->blksize;
}

static void lpps_list_clean( lpps_inode_t **root )
{
	lpps_inode_t *trace = (*root);
	lpps_inode_t *tmp;

	while (trace) {
		tmp = trace;
		trace = trace->next;
		lpps_node_clean(tmp);
	}
}

static lpps_inode_t * create_inode(const char *path)
{
	lpps_inode_t *tmp;
	char * xpath;
	int i;

	tmp = (lpps_inode_t *)os_malloc( sizeof(lpps_inode_t ) );
	if (!tmp) {
		ERR("os_malloc( sizeof(lpps_inode_t) )::failed\n");
		return NULL;
	}

	xpath = (char *)os_malloc( strlen(path) + 1 );
	if (!xpath) {
		os_free(tmp);
		ERR("os_malloc( strlen(%s)+1 )::failed\n",path);
		return NULL;
	}
	strcpy(xpath,path);

	tmp->path 	= xpath;
	tmp->sym	= NULL;
	tmp->uid 	= options.uid;
	tmp->gid 	= options.gid;
	tmp->mtime 	= 
	tmp->ctime	=
	tmp->atime 	= time(NULL);
	tmp->next	= NULL;
	tmp->tuples	= NULL;
	tmp->delta	= NULL;
	tmp->blksize 	= 0;
	tmp->blocks  	= 0;
	tmp->size	= 0;
	tmp->fline	= NULL;
	tmp->rwtasks    = (ppriv **)os_malloc( sizeof(ppriv *) * options.maxtask );
	if (!tmp->rwtasks) {
		ERR(" inode ( %s ) - taskset creation failed\n",path);
		os_free( xpath );
		os_free( tmp );
		return NULL;
	}
	for (i = 0; i < options.maxtask ; i++)
		tmp->rwtasks[ i ] = NULL;

	DBG(" inode ( %s ) create \n",path);

	return tmp;
}


static int lpps_dir_empty( lpps_inode_t * fl )
{
	lpps_inode_t *trace = lpps.root; // definitely root 
	int xlen;

	if (!fl)
		return -1;

	xlen = strlen(fl->path);

	while (trace) {
		if (!strncmp(trace->path,fl->path,xlen) && 
				(trace->path[xlen] == '/')) 
			//something under a node "fl" 
			return 0;
		trace = trace->next;
	}

	return 1;
}

static void lpps_update_time( lpps_inode_t *fl , time_t val )
{
	struct timeval now;

	if (!fl)
		return ;

	gettimeofday( &now, NULL );

	// update time...
	fl->mtime = val ? val : (time_t)(now.tv_sec);
	fl->atime = val ? val : (time_t)(now.tv_sec);
	fl->ctime = val ? val : (time_t)(now.tv_sec);
}

static void lpps_hist_ack( ppriv * item)
{
	int ret;

	if (!item)
		return ;

	if (!item->ackevt)
		return ;

	//
	// Waiting for acknowledgement...
	//
	DBG(" WAIT EVENT %s \n",item->path);
	if (!((ret = os_event_receive( item->ackevt , ACK_RELEASE , 
				options.mrtimeout )) & ACK_RELEASE))
		ERR("   EVT RECEIVE ERR (%s)\n",strerror(ret));		
	if (os_event_delete( item->ackevt ))
		ERR("   EVT DELETE ERR\n");		
	item->ackevt = NULL;

	if (ret & ACK_SHUTDOWN) {
		DBG(" EVT SIGNALLED [%s] \n", item->path);
	} else {
		DBG(" EVT DONE [%s] \n", item->path);
	}
}

//
// adding a history 
//
static ppriv * lpps_hist_add( char * path , int mode )
{
	list_t *trace = lpps.hroot;
	ppriv *item;
	int signalled = 0;
	int locked;

	locked = history_lock();

	do {
		if (!trace)
			break;

		item = parent_of( trace, ppriv, l );
			
		// sanity check 
		if (!item) {
			ERR(" HIST::NULL PRIV \n");
			goto __next_item;
		}
		if (!item->path) {
			//ERR(" HIST::NULL PRIV PATH \n");
			goto __next_item;
		}

		//
		// path compare check 
		//
		if (samepath(item->path,path)) {

			if (item->ackevt) {
				ERR("\n ACKEVT - COLLISION!! %s \n", item->path);
				ERR("FATAL ERROR \n");
				//break;
			} else {
				DBG(" ACKEVT READY %s \n", item->path);
			}

			// setup acknowledge event 
			item->ackevt = os_event_init( (char *)"histevt" );
			if (!item->ackevt) 
				ERR("   EVT ERROR\n");

			DBG(" EVT SEND %s !!\n",item->path);

			// wake up 
			wakeup_hist_task( item, 
				make_hist_block( path, mode) );

			signalled = 1;
			break;
		}

__next_item:
		trace = trace->next;
	} while( lpps.hroot != trace );
	
	history_unlock( locked );

	return signalled ? item : NULL ;
}

//
//   ?json       -> JSON parsing
//   ?wait,delta -> Only deltas are printed 
//
static char * lpps_path_option( char *path, int *option )
{
	char *p;
	char *x, *opt;

	if (!path)
		return NULL;

	if (!(p=strchr(path,PATH_OPTION_DELIM)))
       		return path;

	x = p; //termination 
	if (option) {
		++p;

		// option parsing ...
		opt = strtok(p,PATH_OPTION_SEP);
		while (opt) {
			if (!strcmp(opt,"json")) {
				bit_adds((*option),OPTION_JSON);
			} else
			if (!strcmp(opt,"wait")) {
				bit_adds((*option),OPTION_WAIT);
			} else
			if (!strcmp(opt,"debug")) {
				bit_adds((*option),OPTION_DEBUG);
			} else
			if (!strcmp(opt,"user")) {
				bit_adds((*option),OPTION_USER);
			} else
			if (!strcmp(opt,"delta")) {
				bit_adds((*option),OPTION_DELTA);
			} else 
			if (!strcmp(opt,"deltadir")) {
				bit_adds((*option),OPTION_DELDR);
			} else 
			if ((opt[0] >= '0') && (opt[0] <= '9')) { // isdigit...
				int num = atoi(opt);
				if ((num >= 0) && (num <= 0x7fff))
					bit_adds((*option),(num << 16));
			}
			opt = strtok(NULL,PATH_OPTION_SEP);
		} 
	}
	*x = 0x0;  // null termination 

	return path;
}

static void lpps_log(enum fuse_log_level level, char *fn, const char *fmt, va_list ap )
{
	#define SIZ 2048
	static char lbuf[SIZ];
	int n;

	n = vsnprintf(lbuf,SIZ-1,fmt,ap);
	lbuf[n] = 0x0; // null termination 
	DBG(" <%s> %s",fn, lbuf);
}

//
//
// T O O L K I T    F U N C T I O N S 
//
//
//
static int lpps_create_internal(const char *apath, mode_t mode)
{
	static lpps_inode_t * fl;
	char * path = lpps_path_option( (char *)apath, NULL );

	PATH_CHECK(path)

	DBG(" %s %x \n", path, mode );

	fl = create_inode( (char *)path );
	if (!fl) {
		ERR("create_internal( %s )::failed\n",path);
		return RET_EPERM;
	}

	fl->type = LPPS_FILE;
	fl->mode = S_IFREG | mode;  // file flag 
	#ifdef _QNXAPP_
	//
	// QNX/Linux difference 
	//
	fl->mode &= ~0x800;
	fl->mode |= 0x4;
	#endif
	fl->nlink = 1;
	fl->blksize = BLKSIZE;

	{
		char *p = strrchr((char *)path,'/');
		fl->fline = (char *)os_malloc( strlen(p)+4 );
		if (!fl->fline) {
			ERR("os_malloc( %s+4 )::failed\n",p);
			return RET_EMEM;
		}
		memset( fl->fline, 0, strlen(p)+4 );
		sprintf( fl->fline, print_fmt_fn, p+1 );
	}

	lpps_list_push( &(lpps.root) , fl );

	return 0;
}

//
//
// C A L L B A C K   F U N C T I O N S 
//
//
//

// I N I T 
static void *lpps_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
	DBG("\n");

	//
	// Not sure the following conn-> fields are needed to be initialized like this...
	//
	conn->capable |= FUSE_CAP_EXPORT_SUPPORT; // lookup support 
	conn->capable |= FUSE_CAP_ASYNC_READ;     // Asynchronous read
	conn->capable |= FUSE_CAP_ASYNC_DIO;      // Asynchronous direct I/O
	conn->time_gran = 1000000000;  		  // Nanoseconds
	conn->max_background = options.background;// Max background threads

	//
	// Very important to decrease the size of buffer 
	//
	conn->max_write = 8192; // 16*4 Kbytes
	conn->max_readahead = 32;

	cfg->nullpath_ok  = 0; // nullpath is not ok 

	cfg->kernel_cache = 0; // no kernel cache 

	//
	// ** The most important option ** 
	// "direct_io" should be "1" !!  
	// To support "?wait" option, it is needed. 
	//
	cfg->direct_io    = 1;

	cfg->auto_cache   = 0;  // Not auto cache 

	cfg->remember     = -1; // noforget 

	//
	// debugging
	//  
	cfg->debug = (sys_debug >= DEBUG_FUSE)?1:0;

	return NULL;
}

// G E T A T T R 
static int lpps_getattr(const char *apath, struct stat *stbuf, struct fuse_file_info *fi)
{
	static lpps_inode_t * fl;
	int option = 0;
	char * path = lpps_path_option( (char *)apath, &option );

	PATH_CHECK(path)

	// eliminating useless nodes 
	if (!strncmp(path+1,
			INVALID_FILE,
			strlen(INVALID_FILE))) {
		DBG(" %s SKIP \n", INVALID_FILE);
		return 0;
	}

	LPPS_WLOCK();

	fl = lpps_list_pull( &(lpps.root) , (char *)path );
	if (!fl) {
		LPPS_UNLOCK();
		return RET_ENOENT;
	} 

	stbuf->st_uid   = fl->uid;
	stbuf->st_gid   = fl->gid;
	stbuf->st_atime = fl->atime;
	stbuf->st_ctime = fl->ctime;
	stbuf->st_mtime = fl->mtime;
	stbuf->st_mode  = fl->mode;
	stbuf->st_nlink = fl->nlink;

	//
	// json decoded string requires more space rather than byte string...
	//
	stbuf->st_size  = FILE_SIZE( fl , 
				bit_includes(option,OPTION_JSON)? options.esize : 0 );
	stbuf->st_blksize = fl->blksize;
	stbuf->st_blocks  = fl->blocks;

	lpps_list_push( &(lpps.root) , fl );

#if 0
	// too much noisy ...
	DBG(" %s : bs=%ld, blks=%ld, size=%ld, nlink=%ld, mode=0%x, uid=%d, gid=%d time=%ld \n",
			path, 
                        stbuf->st_blksize,
                        stbuf->st_blocks,
			stbuf->st_size,
                        stbuf->st_nlink,
                        stbuf->st_mode,
                        stbuf->st_uid,
                        stbuf->st_gid,
			stbuf->st_atime);
#endif

	LPPS_UNLOCK();
	return RET_OKAY;
}

// O P E N D I R 
static int lpps_opendir(const char *apath, struct fuse_file_info *fi)
{
	char * path = lpps_path_option( (char *)apath, NULL );

	PATH_CHECK(path)

	DBG(" %s \n", path);

        return 0;
}

// R E A D D I R 
static int lpps_readdir(const char *apath, void *dbuf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags)
{
	lpps_inode_t *trace = lpps.root;
	char * path = lpps_path_option( (char *)apath, NULL );

	PATH_CHECK(path)

	DBG(" %s \n", path);

	LPPS_RLOCK();

	while (trace) {
		if (!trace->path)
			goto readdir_NEXT;

		if (!strncmp(trace->path,path,strlen(path))) {
			struct stat st;

			if (strlen(trace->path) == strlen(path)) {
				trace = trace->next;  // next one 
				continue;
			}

			// filter out the same pattern of beginnig path 
			// a/bb/c - a/bbb/c 
			// aa/b/c - aaa/ 
			if (*(char *)(strrchr(path,'/')+1)) {
				if (*(trace->path + strlen(path)) != '/') {
					trace = trace->next;
					continue;
				}
			}

			//
			// skipping any non-terminal node
			//
			char * objname = trace->path+strlen(path);
			if (*objname == '/') 
				++objname;
			//DBG(" <%s>\n",objname);
			if (strchr(objname,'/')) {
				trace = trace->next;  // non-terminal node 
				continue;
			}

			// terminal node 
			//
			memset( &st, 0, sizeof(st) );
			st.st_uid 	= trace->uid;
			st.st_gid	= trace->gid;
			st.st_atime	= trace->atime;
			st.st_ctime	= trace->ctime;
			st.st_mtime	= trace->mtime;
			st.st_mode	= trace->mode;
			st.st_nlink	= trace->nlink;
			st.st_blksize	= trace->blksize;
			st.st_blocks	= trace->blocks;
			st.st_size	= FILE_SIZE( trace , 0 );  // traditional size 

			if (filler(dbuf, objname, &st, 0, (fuse_fill_dir_flags)0)) {
				ERR("filler(,%s,)::failed\n",trace->path);
				break;
			}
		}
readdir_NEXT:
		trace = trace->next;
	}

	LPPS_UNLOCK();
        return 0;
}

static ppriv * lpps_mkdir_internal(const char *apath, mode_t mode, int *ret)
{
	static lpps_inode_t * fl;
	char * path = lpps_path_option( (char *)apath, NULL );
	ppriv * priv = NULL;

	PATH_CHECK(path)

	DBG(" %s %x \n", path, (int)mode );

	fl = create_inode( (char *)path );
	if (!fl) {
		ERR("lpps_list_pull( %s )::failed\n",path);
		*ret = -1;
		return NULL;
	}

	fl->type = LPPS_DIR;
	fl->mode = S_IFDIR | mode; // directory flag 
	#ifdef _QNXAPP_
	//
	// Qnx app 0666 - can support chdir operation .
	// Meanwhile, linux should include +x option 
	//
	fl->mode |= 0111; // ugo+x 
	#endif
	fl->nlink = 2;
	fl->blksize = BLKSIZE;
	fl->fline = NULL;

	// register to history 
	//
	priv = lpps_hist_add( path , LPPS_HIST_MKDIR );

	// network broadcast 
	lpps_net_multicast( lpps.net.nmid, TRANS_LPPS_MKDIR, path );

	lpps_list_push( &(lpps.root) , fl );

	*ret = 0;
	return priv;
}

// M K D I R 
static int lpps_mkdir(const char *apath, mode_t mode)
{
	ppriv * priv;
	int ret;

	LPPS_WLOCK();
	priv = lpps_mkdir_internal( apath, mode, &ret );
	LPPS_UNLOCK();

	lpps_hist_ack(priv);

	return ret;
}


// C R E A T E 
static int lpps_create(const char *apath, mode_t mode, struct fuse_file_info *fi)
{
	int ret;
	char * path = lpps_path_option( (char *)apath, NULL );
	static lpps_inode_t * fl;
	ppriv *p;
	ppriv *priv = NULL; 

	if (!fi) {
		ERR(" %s - NULL file info \n",path);
		return RET_EIO;
	}

	DBG(" %s %x \n", path, mode );

	LPPS_WLOCK();

	// create a new link 
	ret = lpps_create_internal( path, mode );
	if (ret) {
		ERR("lpps_create( %s )::failed\n",path);
		LPPS_UNLOCK();
		return RET_EIO;
	}

	fl = lpps_list_pull( &(lpps.root) , (char *)path );
	if (!fl) {
		ERR("lpps_list_pull( %s )::failed\n",path);
		LPPS_UNLOCK();
		return RET_ENOENT;
	}

	// creating private structure 
	p = (ppriv *)os_malloc( sizeof( ppriv ) );
	if (!p) {
		ERR( " os_malloc( sizeof(ppriv) ) - failed\n");
		ret = RET_EMEM;
		goto finish_CREATE;
	}

	// initialize 
	p->nth  = 0; // 0th 
	p->rptr = 0;
	p->sig 	= os_sig_init(path);
	p->flags = 0; // flag...
	
	// file handle 
	p->fh   = get_fh(p);
	if (p->fh < 0) {
		ERR(" get_fh() --> %d \n",p->fh);
		os_sig_delete( p->sig );
		os_free( p );	
		ret = RET_EIO;
		goto finish_CREATE;
	}

	// private data structure 
	fl->rwtasks[ p->nth ] = p;
	fi->fh   = p->fh;

	// only if neither .all nor .memory
	//
	if ( lpps_special_paths( path ) == 0 ) {

		// register to history 
		priv = lpps_hist_add( path , LPPS_HIST_MKFILE );

		// network broadcast 
		lpps_net_multicast( lpps.net.nmid, TRANS_LPPS_MKFILE, path );
	}

finish_CREATE:
	lpps_list_push( &(lpps.root) , fl );
	LPPS_UNLOCK();

	// acknowledgement 
	lpps_hist_ack( priv );

	return ret;
}

// R E L E A S E D I R 
static int lpps_releasedir(const char *apath, struct fuse_file_info *fi)
{
	int option = 0;
	char * path = lpps_path_option( (char *)apath, &option );

	DBG(" %s \n",path);

	PATH_CHECK(path)

	return 0;
}

// S Y M L I N K 
static int lpps_symlink(const char *from, const char *to)
{
	static lpps_inode_t * fl;

	DBG(" %s %s \n", from, to);

	LPPS_WLOCK();

	// create a new link 
	if ( lpps_create_internal( to, 0777 ) ) {
		ERR("lpps_create( %s )::failed\n",to);
		LPPS_UNLOCK();
		return RET_EIO;
	}

	fl = lpps_list_pull( &(lpps.root) , (char *)to );
	if (!fl) {
		ERR("lpps_list_pull( %s )::failed\n",to);
		LPPS_UNLOCK();
		return RET_ENOENT;
	}

	// Change attributes.. 
	fl->type = LPPS_LINK;   // type
	fl->mode &= ~S_IFREG;   // not file 
	fl->mode |= S_IFLNK;    // link 
	fl->mode |= 0777;       // mod

	//
	// Deleting first line ...
	//
	if (fl->fline) {
		os_free(fl->fline);
		fl->fline = NULL;
	}

	// removing current symbol, if any... 
	if (fl->sym) 
		os_free( fl->sym );

	// original file name 
	fl->sym = (char *)os_malloc( strlen(from) + 1 );
	if (!fl->sym) {
		ERR("os_malloc( %s+1 )::failed\n",from);
		lpps_node_clean( fl );
		goto finish_SYMLINK;
	}
	memset(fl->sym, 0, strlen(from)+1 );
	strcat(fl->sym, "/"  );
	strcat(fl->sym, from );

	// don't need to think over chaining... 
	fl->size = 0;

	lpps_list_push( &(lpps.root) , fl );

finish_SYMLINK:
	LPPS_UNLOCK();
        return 0;
}

// U N L I N K 
static int lpps_unlink(const char *apath)
{
	char * path = lpps_path_option( (char *)apath, NULL );
	static lpps_inode_t * fl;
	int err = RET_EPERM;
	ppriv *priv = NULL;

	PATH_CHECK(path)

	LPPS_WLOCK();

	DBG(" %s \n", path);

	fl = lpps_list_pull( &(lpps.root) , (char *)path );
	if (!fl) {
		ERR("lpps_list_pull( %s )::failed\n",path);
		err = RET_EEXIST;
		goto finish_UNLINK;
	}

	switch( fl->type ) {
	case LPPS_FILE:
		if ( lpps_special_paths( path ) == 0 ) {
			/* normal file */
			DBG(" %s RMFILE INDICATION \n", path);
			priv = lpps_hist_add( path , LPPS_HIST_RMFILE );
		} else {
			/* ACCTRACE FILE */
			DBG(" %s SPECIAL FILE NO-INDICATION \n", path);
		}

	case LPPS_LINK:

		// clean up private 
		cleanup_priv_tasks( fl );

		// node clean 	
		lpps_node_clean( fl );

		// network broadcast 
		lpps_net_multicast( lpps.net.nmid, TRANS_LPPS_RMFILE, path );

		err = 0;
		break;
	case LPPS_DIR:
	default:
		break;
	}

	if (err)
		lpps_list_push( &(lpps.root) , fl );

finish_UNLINK:
	LPPS_UNLOCK();

	// acknowledgement 
	if (priv)
		lpps_hist_ack( priv );

	DBG(" %s DONE\n", path);

	return err;
}

// R M D I R 
static int lpps_rmdir(const char *apath)
{
	char * path = lpps_path_option( (char *)apath, NULL );
	static lpps_inode_t * fl;
	int err = RET_EPERM;
	ppriv *priv;

	PATH_CHECK(path)

	DBG(" %s \n", path);

	LPPS_WLOCK();

	fl = lpps_list_pull( &(lpps.root) , (char *)path );
	if (!fl) {
		ERR("lpps_list_pull( %s )::failed\n",path);
		LPPS_UNLOCK();
		return err;
	}

	priv = NULL;

	switch( fl->type ) {
	case LPPS_DIR:
		//
		// eliminating .all?deltadir,wait object ...
		//
		if (lpps_dir_empty( fl )) {

			// register to history 
			priv = lpps_hist_add( path , LPPS_HIST_RMDIR );

			// file clean up	
			lpps_node_clean( fl );

			// network broadcast 
			lpps_net_multicast( lpps.net.nmid, TRANS_LPPS_RMDIR, path );

			err = 0;
		}
		break;
	case LPPS_FILE:
	case LPPS_LINK:
	default:
		break;
	}

	if (err)
		lpps_list_push( &(lpps.root) , fl );

	LPPS_UNLOCK();

	if (priv)
		lpps_hist_ack( priv );

	DBG(" %s DONE \n", path);

	return err;
}

// R E N A M E 
static int lpps_rename(const char *from, const char *to, unsigned int flags)
{
	char * path = lpps_path_option( (char *)from, NULL );
	static lpps_inode_t * fl;
	int ret = 0;
	int monitored;

	PATH_CHECK(path)

	DBG(" %s %s %x \n", from, to, flags);

	LPPS_WLOCK();

	// check destination name 
	fl = lpps_list_pull( &(lpps.root) , (char *)path );
	if (!fl) {
		ERR("lpps_list_pull( %s )::failed\n",path);
		LPPS_UNLOCK();
		return RET_EIO;
	}

	// is it link node ??
	if (fl->type == LPPS_LINK) {
		ERR("lpps_list_pull( %s )::failed\n",path);
		lpps_list_push( &(lpps.root) , fl );
		ret = RET_EIO;
		goto finish_RENAME;
	}

	// directory monitored through .all ? 
	monitored = 0;

	// file is opened with "?wait" option?
	if (file_onwait(fl) || monitored) {
		ERR("Now waiting.. ( %s )\n",from);
		lpps_list_push( &(lpps.root) , fl );
		ret = RET_EIO;
		goto finish_RENAME;
	}

	// Now do a shit for renaming 
	switch (fl->type) {
	case LPPS_FILE:
		{
		static lpps_inode_t * tfl;

		// create a target file when required 	
		tfl = lpps_list_pull( &(lpps.root) , (char *)to );
		if (!tfl) {
			if ( lpps_create_internal( to, 0644 ) ) { // create a node 
				ERR("lpps_create_internal( %s ) :: failed \n",to);
				ret = RET_EIO;
				goto finish_RENAME;
			}
			tfl = lpps_list_pull( &(lpps.root) , (char *)to );
		} 

		// file content
		tfl->tuples = lpps_ppsl_duplicate( fl->tuples );

		// update size 
		if (tfl->tuples)
			tfl->size  = tfl->tuples->bytelen;
				
		lpps_list_push( &(lpps.root), tfl );
		}
		break;

	case LPPS_DIR:
		{
		// subfolder / files under original location 
		//  	path change 
		lpps_inode_t *trace = lpps.root;
		int ret;
		char *p;
		int xlen;
		char *xpath;
		//ppriv * priv;

		// create a directory node 
		lpps_hist_ack( 
	        	lpps_mkdir_internal( to, 0755, &ret ) 
		);
		if ( ret ) {
			ERR("lpps_mkdir_internal( %s ) :: failed \n",to);
			// delete node created before 
			ret = RET_EIO;
			goto finish_RENAME;
		}

		while (trace) {
			if ( !strncmp( trace->path , from , strlen(from) ) ) {
				// construct new path name 
				p = trace->path + strlen(from);
				xlen = strlen(to) + strlen(p) + 1;
				xpath = (char *)os_malloc( xlen );
				if (!xpath) {
					ret = RET_EMEM;
					goto finish_RENAME;
				}
				memset( xpath, 0, xlen );
				sprintf( xpath, "%s%s", to, p );
				
				// remove old path name 
				os_free( trace->path );

				// link new name 
				trace->path = xpath;
			}
			trace = trace->next;
		}
		}
		break;
	default:
		break;
	}

	lpps_node_clean( fl );

finish_RENAME:
	LPPS_UNLOCK();
	return ret;
}

// C H M O D 
static int lpps_chmod(const char *apath, mode_t mode, struct fuse_file_info *fi)
{
	char * path = lpps_path_option( (char *)apath, NULL );

	PATH_CHECK(path)

	DBG(" %s %x \n", path, mode);

	return 0;
}

// C H O W N 
static int lpps_chown(const char *apath, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
	char * path = lpps_path_option( (char *)apath, NULL );

	PATH_CHECK(path)

	DBG(" %s %x %x \n", path, uid, gid );

	return 0;
}

// F L U S H 
static int lpps_flush(const char *apath, struct fuse_file_info *fi)
{
	int option = 0;
	char * path = lpps_path_option( (char *)apath, &option );
	int ret = 0;
        ppriv * priv;

	if (!apath)
		ERR(" PATH=NULL\n");

	PATH_CHECK(path)

        //
        // rewinding back to ... 
        //
        priv = (ppriv *)get_priv( fi->fh );  // private structure
        if ( !bit_includes(priv->flags,OPTION_WAIT) )
                priv_rewind(priv);

	DBG(" %s \n", path );

	return ret;
}

// U T I M E N S 
static int lpps_utimens(const char *apath, const struct timespec tv[2], struct fuse_file_info *fi)
{
	char * path = lpps_path_option( (char *)apath, NULL );
	static lpps_inode_t * fl;
	time_t asec = (time_t)(tv[0].tv_sec);
	time_t msec = (time_t)(tv[1].tv_sec);

	PATH_CHECK(path)

	DBG(" %s %d %d \n", path, (int)asec, (int)msec );

	LPPS_RLOCK();

	fl = lpps_list_peek( &(lpps.root) , (char *)path );
	if (!fl) {
		ERR("lpps_list_pull( %s )::failed\n",path);
		LPPS_UNLOCK();
		return RET_EEXIST;
	}

	lpps_update_time( fl , asec );

	LPPS_UNLOCK();
	return 0;
}

// O P E N 
static int lpps_open(const char *apath, struct fuse_file_info *fi)
{
	int option = 0;
	char * path = lpps_path_option( (char *)apath, &option );
	static lpps_inode_t * fl;
	int err = 0;
	int i = 0;
	int memsz;
	ppriv * p = NULL;

	PATH_CHECK(path)

	/* special file checks 
	 * - No more two instances cannot alive....
	 * 
	 */
	if ((lpps_special_paths(path) == ACCTRACE_FILE) && 
			(bit_includes(option,OPTION_WAIT|OPTION_DELDR))) {
		/* special file */
		if ( priv_hist_search( path ) ) {
			ERR(" %s IS RUNNING \n",path);
			return RET_EEXIST;
		}
	}
	
	LPPS_WLOCK();

	//
	// If current total amount of free memory is smaller than "bmemory = ",
	//   then file open would keep being failed. 
	//
	// options.bmemory keeps a value from "bmemory = " in lpps.conf. 
	// Good number for this variable is not easy to be determined becuase 
	// we cannot easily know how biggest block from LIBFUSE is tried to be allocated. 
	// In experiment, we recommend "1024" since it looks that at least 512Kbytes 
	// would be allocated from the LIBFUSE. Of course, the experiment was conducted from 
	// ubuntu 20.04 Linux desktop. 
	//
	memsz = os_meminfo(NULL,0,NULL,1);
	if ( KILO(options.bmemory) > memsz ){
		ERR(" SYSTEM TOO MUCH LOW MEMORY %dKbytes \n", memsz / KILO(1) );
		LPPS_UNLOCK();
		return RET_EMEM;
	}

	fl = lpps_list_pull( &(lpps.root) , (char *)path ); 
	if (!fl) {
		LPPS_UNLOCK();
		return RET_EEXIST;
	}

	// allocate 
	for (; i < options.maxtask; i++) {
		if (fl->rwtasks[ i ])
			continue;
		break;
	}

	if (i >= options.maxtask) {
		ERR( " No empty rwtasks map in %s \n", path);
		err = RET_EMEM;
		goto finish_OPEN;
	}

	// creating private structure 
	p = (ppriv *)os_malloc( sizeof( ppriv ) );
	if (!p) {
		ERR( " os_malloc( sizeof(ppriv) ) - failed\n");
		err = RET_EMEM;
		goto finish_OPEN;
	}

	// initialize - priv structure 
	p->nth  = i;
	p->rptr = 0;
	p->flags = 0; // flag...
	p->sig 	= os_sig_init(path);

	// getting a new file handle 
	p->fh = get_fh(p);
	if (p->fh < 0) {
		ERR(" get_fh() --> %d \n",p->fh);
		os_sig_delete( p->sig );
		os_free( p );	
		err = RET_EIO;
		goto finish_OPEN;
	}

	//
	// Json buffer size 
	//
	p->buflen = KILO( options.json_buflen ); /* K bytes */

	//
	// linebuffer - please look into lpps_ppsl_dump() 
	// TODO. This is only used for read transaction. 
	//       We need to differentiate open access mode ? 
	//       If it does not include "read", then skip it ?
	//
	p->linebuf = (char *)os_malloc( p->buflen );
	if (!p->linebuf) {
		ERR(" linebuf creattion error --> %d \n",p->fh);
		os_sig_delete( p->sig );
		put_fh( p->fh );
		os_free( p );	
		err = RET_EMEM;
		goto finish_OPEN;
	}

	//
	// history special file initialization 
	//
	list_init( &(p->l) );
	p->path    = NULL;
	p->hq.blks = NULL;
	p->hq.size = 0;

	p->ackevt  = NULL;

	//	
	// duplicating option...
	//
	// delta option needs to be inferred at "write" side.. 
	// 
	if (bit_includes(option,OPTION_WAIT))
		bit_adds(p->flags,OPTION_WAIT);

	if (bit_includes(option,OPTION_DELTA))
		bit_adds(p->flags,OPTION_DELTA);

	if (bit_includes(option,OPTION_JSON))
		bit_adds(p->flags,OPTION_JSON);

	if (bit_includes(option,OPTION_DELDR))
		bit_adds(p->flags,OPTION_DELDR);

	if (bit_includes(option,OPTION_USER))
		bit_adds(p->flags,OPTION_USER);

	if (bit_includes(option,OPTION_DEBUG))
		bit_adds(p->flags,OPTION_DEBUG);

	//
	// private data structure 
	fi->fh   = p->fh;

	//
	// registered to rwtasks structure
	//
	fl->rwtasks[ i ] = p;

	//
	// Log...
	//
	DBG(" <%s> [ %d ] <%d> %s %s %s %s \n", path, p->nth, (int)fi->fh,
			bit_includes(p->flags,OPTION_WAIT)?"wait":"non-block",
			bit_includes(p->flags,OPTION_JSON)?"json":"normal",
			bit_includes(p->flags,OPTION_DELTA)?"delta":"full",
			bit_includes(p->flags,OPTION_DELDR)?"deltadir":"file"
	   );

finish_OPEN:
	lpps_list_push( &(lpps.root) , fl ); 

	LPPS_UNLOCK();
	return err;
}

// R E L E A S E 
static int lpps_release(const char *apath, struct fuse_file_info *fi)
{
	int option = 0;
	char * path = lpps_path_option( (char *)apath, &option );
	lpps_inode_t *fl;
	int ret = 0;
	int fh = -1;
	int nth = -1;
	int fh_in_priv; 
	ppriv *priv;

	if (!apath)
		ERR(" PATH=NULL\n");

	PATH_CHECK(path)

	LPPS_WLOCK();

	fl = lpps_list_peek( &(lpps.root) , (char *)path );
	if (!fl) {
		LPPS_UNLOCK();
		return RET_EEXIST;
	}

	if (!fi) {
	       ERR(" %s - NULL fi \n",(char *)path);
	       goto finish_RELEASE;
	}

	//
	// It is assigned in lpps_open(). 
	fh = fi->fh; // file handle  

	priv = (ppriv *)get_priv( fh );
	if (!priv) {
		ERR(" %s - NULL priv for %d \n",(char *)path, fh );
		goto finish_RELEASE;
	}

	//
	// Priv structure ...
	//
	fh_in_priv = priv->fh;

	if (fh_in_priv != fh) { // sanity check again 
		ERR(" %s - file handle mismatch %d/%d \n", (char *)path, fh_in_priv, fh);
		ret = RET_EIO;	
		goto finish_RELEASE;
	}

	nth = priv->nth; // location of DB 

	if (!((nth >= 0) & (nth < options.maxtask))) { // sanity check again
		ERR(" %s - DB location index error %d\n", (char *)path, nth);
		ret = RET_EIO;	
		goto finish_RELEASE;
	}

	if (fl->rwtasks[ nth ] != priv) // sanity check 
		ERR(" %s - nth DB has broken sync at %d \n",path,nth);

	// nullifying 
	fl->rwtasks[ nth ] = NULL;

	//
	// Please refer to cleanup_priv_tasks() function. 
	// The following few steps are very similar to major steps in that function. 
	//
	
	// signal delete..
	os_sig_delete( priv->sig );

	// deleting linebuffer 
	if (priv->linebuf)
		os_free( priv->linebuf);

	put_fh( fh ); // returning back file handle 

	//
	// Event sending.. now stop me !! 
	//
	if (priv->ackevt) {
		DBG(" %s DONE + WRITE STOP !! \n", (char *)path);
		os_event_send( priv->ackevt , ACK_RELEASE ); 
	}

	// remove 
	os_free( priv );

	DBG(" %s [ %d ] <%d> \n", path, nth, fh );

finish_RELEASE:
	LPPS_UNLOCK();
	return ret;
}

// R E A D L I N K 
static int lpps_readlink(const char *apath, char *linkbuf, size_t size)
{
	char * path = lpps_path_option( (char *)apath, NULL );
	lpps_inode_t *fl;

	PATH_CHECK(path)

	DBG(" %s %d \n", path , (int)size );

	LPPS_RLOCK();

	fl = lpps_list_peek( &(lpps.root) , (char *)path );
	if (!fl) {
		DBG(" NOT EXIST (%s) \n", path);
		LPPS_UNLOCK();
		return RET_EEXIST;
	}

	//
	// Symbolic link process. 
	// # ln -s <original> <target> 
	//
	
	if (!fl->sym) {
		ERR("NULL SYM LINK (%s) \n",path);
		memset( linkbuf, 0, size );
	} else 
		strncpy( linkbuf, fl->sym+1, size );

	LPPS_UNLOCK();
	return 0;
}

// R E A D 
static int lpps_read(const char *apath, char *rbuf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int option = 0;
	char * path = lpps_path_option( (char *)apath, &option );
	lpps_inode_t *fl;
	int filesize;
	int maxamt = 0;
	int dumpamt = 0;
	int fh;
	int eof;
	ppriv * priv;
	int stype;

	PATH_CHECK(path)

	if (!fi) {
		ERR(" %s - NULL file info !!\n",(char *)path );
		return RET_EIO;
	}

	LPPS_RLOCK();

	fl = lpps_list_peek( &(lpps.root) , (char *)path );
	if (!fl) {
		ERR(" NOT EXIST (%s) \n", path);
		LPPS_UNLOCK();
		return RET_EEXIST;
	}

	//
	// shouldn't be directory
	//
	if (fl->type == LPPS_DIR) {
		ERR(" NOT FILE (%s) \n", path);
		LPPS_UNLOCK();
		return RET_EIO;
	}

	//
	// file handle & private structure
	//
	fh = fi->fh;            
	priv = (ppriv *)get_priv( fh );  // private structure
	if (!priv) {
		ERR(" %s - NULL private structure !!\n",(char *)path );
		LPPS_UNLOCK();
		return RET_EIO;
	}

	if (priv->fh != fh) {   // sanity check
		ERR(" %s - file handle mistmatch %d-%d !!\n",(char *)path,priv->fh,fh);
		LPPS_UNLOCK();
		return RET_EIO;
	}

	//
	// Processing special pathname ".memory" ".all"
	//
	if ((stype = lpps_special_paths(path)) != 0) {
		//int len = 0;
		int amt = 0;

		if (offset != 0) {  // second round - just return 0 "terminate.."
			if ((stype == MEMTRACE_FILE) || 
					!bit_includes(priv->flags,OPTION_WAIT|OPTION_DELDR)) {
				LPPS_UNLOCK();
				return 0;
			}
		}

		switch (stype) {
		case MEMTRACE_FILE:
			//
			// # cat .memory 
			//
			if (bit_includes(priv->flags,OPTION_DEBUG)) {
				int loc = option >> 16; // upper 16 bits..
				char * p;
				int psz;

				p = os_memblock(loc);   // memory start block 
				psz = os_memsize(loc); // memory block size

				amt = p ? lpps_dumpmem(rbuf,size,p,psz) : 0;
			} else 
			if (bit_includes(priv->flags,OPTION_USER)) {
				int count = 0;

				memset(rbuf,0,size);
				snprintf(rbuf,size,"------- %d \n", os_meminfo(NULL,0,NULL,1));
				amt = strlen(rbuf);
				{
					list_t * trace = lpps.hroot;
					ppriv * priv;
					do {
						if (!trace)
							break;
						priv = parent_of( trace, ppriv, l );
						snprintf(rbuf+amt,size-amt,"<%s> \n",priv->path);
						amt = strlen(rbuf);
						trace = trace->next; 
						count ++;
					} while ( lpps.hroot != trace );
				}
				snprintf(rbuf+amt,size-amt,"%d \n", count);
				amt = strlen(rbuf);
			} else {
				snprintf(rbuf,size,"%d\n",os_meminfo(NULL,0,NULL,1));
				int hlen = strlen(rbuf);

				amt = 0;
				os_meminfo(rbuf+hlen,size-hlen,&amt,2);
				amt += hlen; // amount of data dumped from ... 
			}
			LPPS_UNLOCK();
			return amt;

		case ACCTRACE_FILE:
			if (!bit_includes(priv->flags,OPTION_WAIT|OPTION_DELDR)) {
				ERR(" <%s> ACCTRACEFILE - only DELDR | WAIT is supported (%x) \n",apath,priv->flags);
				LPPS_UNLOCK();
				return RET_EIO; // we don't support accumulated history
			} 
			break;
		}	
	}

	//
	// Now read...
	//
	priv->mode = READ_WORK;

	//
	// this is the amount of allocation inside of libfuse 
	// 
	filesize = FILE_SIZE( fl , 
		       bit_includes(priv->flags,OPTION_JSON)? options.esize : 0 );

	//
	// first line processing
	// 
	if ((offset == 0) && fl->fline)
		dumpamt = sprintf(rbuf,"%s",fl->fline);

	// 
	// end of file check - 
	// Depending on the transaction type, when too many records are to be stacked up 
	// in this file, it is needed to be checked. 
	if (fl->tuples)
		eof = priv_eof(fl->tuples,priv)? 1 : 0;
	else
		eof = 1;

	//
	// "?delta" option - overriding EOF
	//
	if (bit_includes(priv->flags,OPTION_DELTA))
		eof = 1;

	DBG(" %s %d %d/%d <%d> %s %s %s %s \n", path , (int)offset , (int)size, filesize, (int)fh, 
			bit_includes(priv->flags,OPTION_WAIT)?"wait":"non-block",
			bit_includes(priv->flags,OPTION_JSON)?"json":"normal",
			bit_includes(priv->flags,OPTION_DELTA)?"delta":"full", 
			bit_includes(priv->flags,OPTION_DELDR)?"deltadir":"norm"  );

	if (eof) { // check beginning of data 
		DBG(" %s - REACH TO AN END %d > %d \n", (char *)path, (int)offset, (int)filesize );

		if ( !bit_includes(priv->flags,OPTION_WAIT) ) {
			DBG(" %s - TERMINATE %-4d \n", (char *)path, fi->unique );
			LPPS_UNLOCK();
			return dumpamt; // read transaction end !!
		} else {
                        // 02122023 - Todd 
                        //
                        // Rewinding at only the case of WAITING state...
                        //
                        priv_rewind(priv);
			DBG(" %s - WAIT OPTION INCLUDED %-4d \n", (char *)path, fi->unique );
		}
	}

	// debugging...
	// lpps_ppsl_print( fl->tuples );

	if ( bit_includes(priv->flags,OPTION_WAIT) && eof ) {

		// signalling unique # 
		priv->unique = fi->unique; 

		//
		// Write/Read shouldn't be overlapped. 
		// Write/Read on the same file works in execulsive manner through hand-shake mechanism
		//
		if (priv->ackevt) {
			DBG(" ackevt SEND %s !!\n",path);
			os_event_send( priv->ackevt , ACK_RELEASE );
		}

		if (stype) { /* special file */
			if ( !priv_hist_setup( priv, path ) ) {
				DBG(" %s SPECIAL FILE--> ADD \n", path);
			} else {
				DBG(" %s SPECIAL FILE--> WAIT \n", path);
			}
		} 

		//
		// Let other tasks be able to obtain the lock 
		//
		LPPS_UNLOCK();

		if ( !os_sig_wait( priv->sig , 0 ) ) {

			DBG(" SIGNALLED %s \n",path);

			//
			// Required for exclusive access between read and write...
			//
			LPPS_RLOCK();

			//
			// First order check !! 
			//
			if ( bit_includes(priv->sig->state,OS_LOCK_STATE_DONE) ){

				int normfile;
				int intr = bit_includes(priv->sig->state,OS_LOCK_STATE_INTR);

				//
				// following bit check will not be performed 
				//
				bit_clear(priv->sig->state);
			
				//
				// OS_LOCK_STATE_WRITE and all other states are invalidated. 
				//
				dumpamt = 0;
		
				// rewind to the beginning of the file 
				// End of read transaction !! 
				priv_rewind(priv);

				//
				// Deleting history... 
				//
				normfile = priv_hist_delete( priv );

				DBG(" SIGNALLED %s BY STATE_DONE %s %s \n", path, 
						normfile ? "NORMAL" : "SPECIAL" ,
						intr ? "INTR" : "NORM" );

				if (!normfile && !intr) {
					DBG(" PRIV DELETE\n");
					os_free( priv );
				}

				LPPS_UNLOCK();

				return 0; /* DONE HERE !! */
			}

			//
			//
			// This is .all file 
			//
			if (priv_hist_enabled(priv)) {
				if (lpps_special_paths(priv->path) == ACCTRACE_FILE) {
					lpps_hist_blk_t * blk;
					const char *fmt;
					do {
						blk = hist_queue_get( &priv->hq );
						if (!blk) 
							break;

						if (!blk->path && (blk->mode == LPPS_HIST_NULL)) {
							DBG("NULL SIGNAL RECEIVE\n");
							remove_hist_block( blk );
							dumpamt = 0;
							break;
						}

						fmt = hist_print_fmt(blk->mode);

						// dump to buffer 
						char *path = strrchr(blk->path,'/')+1;
						memset(rbuf,0,(strlen(path)+10)>size?size:strlen(path)+10);
						snprintf(rbuf,size,fmt,path);

						DBG(" MESSAGE=[%s]\n",rbuf);

						// result length 
						dumpamt = strlen(rbuf);

						remove_hist_block( blk );
					} while (1);
				}
			}

			// wait signal 
			// 	1. external write event 
			// 	2. CTRL-C exception 
			if ( bit_includes(priv->sig->state,OS_LOCK_STATE_WRITE) ){

				// first line
				dumpamt = sprintf(rbuf,"%s",fl->fline);

				//  * getting the size of file again 
				//  -- the size of file might differ due to write transaction
				//  
				filesize = FILE_SIZE( fl ,
		       				bit_includes(priv->flags,OPTION_JSON)? options.esize : 0 );

				maxamt = (size > filesize) ? filesize : size;
				maxamt -= dumpamt; // first line 

				DBG(" WRITE SIGNALLED [ %d ] - %d %d %ld \n", tuples_size(fl->tuples), filesize, maxamt, size );

				if (bit_includes(priv->flags,OPTION_DELTA) && 
						fl->delta ) {  // fl->delta is filled up from lpps_write() - lpps_ppsl_merge() 
					DBG(" %s DELTA OPTION !! < %d > \n",(char *)path, dumpamt);

					//
					// Simply returning the maximum value to stop read job 
					// It is required when we use "?delta,json" option 
					//
					dumpamt += lpps_ppsl_dump( fl->delta, rbuf+dumpamt, maxamt, 
								bit_includes(priv->flags,OPTION_JSON), priv );

					dumpamt = maxamt; // flushing...
				} else {
					dumpamt += lpps_ppsl_dump( fl->tuples, rbuf+dumpamt, maxamt, 
								bit_includes(priv->flags,OPTION_JSON), priv );
				}
				bit_excludes(priv->sig->state,OS_LOCK_STATE_WRITE);
			} 

			LPPS_UNLOCK(); 

			os_sig_unlock( priv->sig );
		}
		//priv->unique = 0; - removed
	} else {
		//
		// buffer size calculation 
		//
		maxamt = (size > filesize) ? filesize : size;
		maxamt -= dumpamt; // first line 

		// 
		//  Dump the contents of file
		//
		dumpamt += lpps_ppsl_dump( fl->tuples, rbuf+dumpamt, maxamt, 
						bit_includes(priv->flags,OPTION_JSON), priv );
		LPPS_UNLOCK();
	}

	DBG(" size=%d ret=%d | %d:%d -> %d \n", fl->size, lpps_res_size(fl), (int)offset, maxamt, dumpamt );

	return dumpamt;
}

// W R I T E 
static int lpps_write(const char *apath, const char *wbuf, size_t size, off_t offset, 
			struct fuse_file_info *fi)
{
	char * path = lpps_path_option( (char *)apath, NULL );
	lpps_inode_t *fl;
	int adjamt = 0;
	ppsl *curl = NULL;
	int i;
	ppriv *priv;
	ppriv *xpriv; // myself 
	int fh; 

	PATH_CHECK(path)

	if (!fi) {
		ERR(" %s - NULL file info !!\n",(char *)path );
		return RET_EIO;
	}

	// 
	//  check file type 
	//
	{
		int file_type;
		if ((file_type = lpps_special_paths(path)) != 0) {
			switch (file_type) {
			case MEMTRACE_FILE:
			case ACCTRACE_FILE:
				DBG(" CANNOT WRITE THIS FILE %s \n",path);
				return size;
			}
		} 
	}

	LPPS_WLOCK();

	fl = lpps_list_pull( &(lpps.root) , (char *)path );
	if (!fl) {
		ERR(" NOT EXIST (%s) \n", path);
		LPPS_UNLOCK();
		return RET_EEXIST;
	}

	DBG(" %s %d %d \n", path , (int)offset , (int)size );

	//
	// shouldn't be directory
	//
	if (fl->type == LPPS_DIR) {
		ERR(" NOT FILE (%s) \n", path);
		lpps_list_push( &(lpps.root) , fl );
		LPPS_UNLOCK();
		return RET_EIO;
	}

	//
	// file handle & private structure
	//
	fh = fi->fh;            // file handle 
	priv = (ppriv *)get_priv( fh );  // private structure
	if (!priv) {
		ERR(" %s - NULL private structure !!\n",(char *)path );
		lpps_list_push( &(lpps.root) , fl );
		LPPS_UNLOCK();
		return RET_EIO;
	}

	if (priv->fh != fh) {   // sanity check
		ERR(" %s - file handle mistmatch %d-%d !!\n",(char *)path,priv->fh,fh);
		lpps_list_push( &(lpps.root) , fl );
		LPPS_UNLOCK();
		return RET_EIO;
	}

	//
	// Now write ...
	//
	priv->mode = WRITE_WORK;

	// parse lists...
	curl = NULL;
	curl = lpps_ppsl_parse( curl, wbuf, offset, size );
	if (!curl) {
		ERR(" lpps_pps_parse( %s, size=%d ):error\n", (char *)path, (int)size);
		lpps_update_time( fl , 0 ); // update time... 
		lpps_list_push( &(lpps.root) , fl );
		LPPS_UNLOCK();
		//
		// The control will fall into here just in case that we might run "vi" editor under LPPS partition. 
		// Simply returning back to the upper control as if "wbuf" data has been successfully written to file.
		//
		//return RET_EEXIST;
		//
		// FAKE RETURN 
		return size;
	}

	// TODO - writing mode plan 
	if (fl->tuples && (offset == 0)) {
		DBG(" %s - file cleanup \n", (char *)path );
		lpps_ppsl_remove( &(fl->tuples) );
		fl->tuples = NULL;
	}
	
	// merging two lists...
	if (!fl->tuples) {
		fl->tuples = curl;
		{
			int i = 0;
			ppsitem *item;
			while( i < fl->tuples->occupancy ) {
				item = (ppsitem *)&(fl->tuples->item[i]);
				DBG(" add [ %s:%s:%s ] \n",item->attrib,item->type,item->value);
				++ i;
			}
		}
	} else {
		//
		// fl->tuples --> total tuples
		// fl->delta --> only latest tuples 
		//
		fl->tuples = lpps_ppsl_merge( fl->tuples, curl, &fl->delta ); 
		lpps_ppsl_remove( &(curl) );

		if (!fl->tuples) {
			ERR(" lpps_ppsl_merge():error\n");
			lpps_list_push( &(lpps.root) , fl );
			LPPS_UNLOCK();
			return RET_EMEM;
		}
	}

	// debugging ...
	// lpps_ppsl_print( fl->tuples );
	// if (fl->delta)
	//	lpps_ppsl_print( fl->delta );
	
	// update size...
	fl->size = fl->tuples->bytelen;

	// sorting...
	if (fl->tuples)
		lpps_ppsl_sort( fl->tuples, 0, tuples_size(fl->tuples)-1 );

	{
		// memory information 
		// 
		int memsz = os_meminfo(NULL,0,NULL,1);
		int mbytes;
		int kbytes;

		mbytes = memsz / MEGA(1);
		kbytes = memsz - (mbytes * MEGA(1));

		DBG(" size=%d ret=%d adjamt=%d %d.%d Mbytes \n",
			fl->size, lpps_res_size(fl), adjamt, mbytes, kbytes);
	}

	// debugging...
	// lpps_ppsl_print( fl->tuples );

	lpps_update_time( fl , 0 ); // update time... 

	lpps_list_push( &(lpps.root) , fl );

	// signalling
	for (i = 0; i < options.maxtask; i++) { 
		xpriv = fl->rwtasks[ i ];
		if (!xpriv)
			continue;
		if (priv == xpriv)
			continue;
		if (xpriv->mode != READ_WORK)
			continue;
		if (!signal_waiting(xpriv->sig))
	       		continue;

		// lock should be obtained 
		os_sig_lock( xpriv->sig );
		bit_adds(xpriv->sig->state,OS_LOCK_STATE_WRITE);

		//
		// Creates and establishes an event for all reader process
		xpriv->ackevt = os_event_init( (char *)"waitevt" );
		if (!xpriv->ackevt) 
			ERR("   EVT ERROR\n");

		// lock should be obtained 
		os_sig_wakeup( xpriv->sig , 0 );
		DBG(" %s - WAKEUP %d \n", (char *)path, i);
	}

	//
	// Until the final step, all the read transaction simply awaits...
	//
	LPPS_UNLOCK();

	//
	// Readers kicks down the ground and goes running. 
	//
	
	// termination steps 
	for (i = 0; i < options.maxtask; i++) { 
		xpriv = fl->rwtasks[ i ];
		if (!xpriv)
			continue;
		if (priv == xpriv) 
			continue;
		if (xpriv->mode != READ_WORK)
			continue;

		// gets signal from read transaction 
		if (xpriv->ackevt) {
			int xret;
			DBG(" WAIT EVENT %d \n",i);
			if (!((xret = os_event_receive( xpriv->ackevt , ACK_RELEASE , 
					options.mrtimeout )) & ACK_RELEASE))
				ERR("   EVT RECEIVE ERR (%s)\n",strerror(xret));		
			if (os_event_delete( xpriv->ackevt ))
				ERR("   EVT DELETE ERR\n");		
			xpriv->ackevt = NULL;

			if (xret & ACK_SHUTDOWN) {
				DBG(" EVT SIGNALLED [%d] \n", i);
				return 0; // no more write will be performed !! 
			} else
				DBG(" EVT DONE [%d] \n", i);
		}
	}

	return size;
}

// S T A T F S 
static int lpps_statfs(const char *apath, struct statvfs *buf)
{
	char * path = lpps_path_option( (char *)apath, NULL );

	PATH_CHECK(path)

	DBG(" %s \n", path );

	return 0;
}

//
//
//
// M A I N 
//
//
//

static int parse_option( void *data , const char *arg, int key, struct fuse_args *outargs )
{
	switch (key) {
	case FUSE_OPT_KEY_NONOPT:
		if (!lpps.mntpt) {
		    	lpps.mntpt = strdup(arg);
			if (!lpps.mntpt) {
				ERR("os_strdup( %s )::failed\n",arg);
				exit(1);
			}
			DBG(" MNT %s\n",lpps.mntpt);
		}
		break;
	default:
		//
		// skipping 
		break;
	}

	return 0;
}

int main(int argc,char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	int ret;

	// initializing
	lpps_oper.init		= lpps_init;
	lpps_oper.getattr	= lpps_getattr;
	lpps_oper.opendir	= lpps_opendir;
	lpps_oper.readdir	= lpps_readdir;
	lpps_oper.releasedir	= lpps_releasedir;
	lpps_oper.readlink	= lpps_readlink;
	lpps_oper.mkdir		= lpps_mkdir;
	lpps_oper.symlink	= lpps_symlink; 
	lpps_oper.unlink	= lpps_unlink;
	lpps_oper.rmdir		= lpps_rmdir;
	lpps_oper.rename	= lpps_rename;
	lpps_oper.chmod		= lpps_chmod;
	lpps_oper.chown		= lpps_chown;
	lpps_oper.flush		= lpps_flush;
	lpps_oper.utimens	= lpps_utimens;
	lpps_oper.open		= lpps_open;
	lpps_oper.read		= lpps_read;
	lpps_oper.write		= lpps_write;
	lpps_oper.release	= lpps_release;
	lpps_oper.statfs	= lpps_statfs;
	lpps_oper.create	= lpps_create;

	lpps.oper = &lpps_oper;

	//
	// fuse option processing... 
	//
	options.user = strdup( "root" );
	options.conf = strdup( "/etc/lpps.conf" );
	options.daemon = 0; /* foreground by default */
	fuse_opt_parse(&args, &options, option_spec, parse_option );

	if (options.daemon)
		openlog("lpps", LOG_PID|LOG_CONS, LOG_USER);

	fuse_set_log_func( (fuse_log_func_t)lpps_log );

	// 
	// configuration file loading...
	//
	lpps.conf = conf_read( options.conf );
	if (!lpps.conf) {
		ERR(" Configuration %s does not exist.\n",options.conf);
		return 0;
	}

	//
	// Configuration value reading...
	//
	options.memory      = conf_ival( lpps.conf, CONF_MEMSPC ); // system memory
	options.bmemory     = conf_ival( lpps.conf, CONF_MEMBOT ); // memory bottom line
	options.json_buflen = conf_ival( lpps.conf, CONF_JSONBUF ); // json buffer length 
	options.debug       = conf_ival( lpps.conf, CONF_DEBUG );  // debug mode 
	options.esize       = conf_ival( lpps.conf, CONF_FILESIZE ); 
	options.mshare      = conf_ival( lpps.conf, CONF_MAXSHARE ); 
	options.background  = conf_ival( lpps.conf, CONF_FUSEBK ); 
	options.mrtimeout   = conf_ival( lpps.conf, CONF_MRTIMEOUT );
	options.maxtask     = conf_ival( lpps.conf, CONF_MAXTASK );
	options.fn_memtrace = conf_sval( lpps.conf, CONF_MEMTRACE ); // memory trace name 
	options.fn_acctrace = conf_sval( lpps.conf, CONF_ACCTRACE ); // access trace name 
	options.history     = conf_ival( lpps.conf, CONF_HISTSIZE ); // history size
	options.network     = conf_ival( lpps.conf, CONF_NETWORK ); // multicast use or not 
	options.ip          = conf_sval( lpps.conf, CONF_NETIP   ); 
	options.port        = conf_ival( lpps.conf, CONF_NETPORT ); 
	options.netid       = conf_ival( lpps.conf, CONF_NETID   );  
	options.netdev      = conf_sval( lpps.conf, CONF_NETDEV  );  

	/* transport */
	{
		char *tp = conf_sval( lpps.conf, CONF_TRANSPORT); // udpv4/udpv6/ethernet

		if (!strcmp(tp,"udp4"))
			options.transport = TRANS_UDP_IPV4;
		else if (!strcmp(tp,"udp6"))
			options.transport = TRANS_UDP_IPV6;
		else if (!strcmp(tp,"raw"))
			options.transport = TRANS_IEEE_802_3;
		else 
			options.transport = TRANS_UDP_IPV4;
	}

	//
	// Configuration value copying...
	//
	sys_rmem = MEGA( options.memory );
	sys_debug = options.debug;
	sys_daemon = options.daemon;
	use_syslog = options.syslog;

	if (sys_debug >= DEBUG_FUSE) {	
		if( fuse_opt_add_arg(&args, "-d" ) == -1 ) {
			ERR("fuse_opt_add_arg( %s )::failed\n","-d");
			exit(1);
		}
	}

	if( fuse_opt_add_arg(&args, "-o" DEFAULT_ARGS ) == -1 ) {
		ERR("fuse_opt_add_arg( %s )::failed\n",DEFAULT_ARGS);
		exit(1);
	}

	lpps.fuse = fuse_new(&args, lpps.oper, sizeof(struct fuse_operations), NULL);
	if(lpps.fuse == NULL) {
		ERR("fuse_new() failed\n");
		exit(1);
	}

	lpps.root  = NULL; // inode root for normal files/folders

	if (!lpps.mntpt) 
		lpps.mntpt = strdup(TMP_MNTPOINT);

	lpps.mut  = os_rwlock_init();  // main operation lock

	lpps.hist_lock = os_sig_init( "HISTLOCK" );  // history tracking lock 

	//
	// History transaction node initialization 
	lpps.hroot   = NULL;

	//
	// Option output...
	//
	ERR(" ----------------------------------------\n");
	ERR(" MOUNT POINT : %s \n",lpps.mntpt);
	ERR(" USER NAME   : %s \n",options.user);
	ERR(" CONFIG FILE : %s \n",options.conf);
	ERR(" MEMORY      : %dM\n",options.memory);
	ERR(" DEBUG LVL   : %d \n",options.debug);
	ERR(" EXPAND      : %d \n",options.esize);
	ERR(" CONCURRENCY : %d \n",options.mshare);
	ERR(" MAX TASKS   : %d \n",options.maxtask);
	ERR(" ACK TIMEOUT : %d \n",options.mrtimeout);
	ERR(" FUSE BKGND  : %d \n",options.background );
	ERR(" MEMTRACE    : %s \n",options.fn_memtrace);
	ERR(" ACCTRACE    : %s \n",options.fn_acctrace);
	ERR(" MEMORY BOT  : %dK\n",options.bmemory);
	ERR(" TPL BUFFER  : %dK\n",options.json_buflen);
	ERR(" NETWORK     : %s\n", options.network?"YES":"NO");
	ERR(" MCAST IP    : %s\n", options.ip);
	ERR(" MCAST PORT  : %d\n", options.port);
	ERR(" TRANSPORT   : %d\n", options.transport);
	ERR(" NET ID      : %d\n", options.netid);
	ERR(" INTERFACE   : %s\n", options.netdev);
	ERR(" ----------------------------------------\n");

	// getting current user id & group id
	// We used commands; "id -u xxx" and "id -g xxx"
	//
	do {
		char cmd[64];
		char ugid[16]; 

		// user ID 
		memset(ugid, 0, 16);
		memset(cmd, 0, 64);
		sprintf(cmd,"id -u %s",options.user);
		if (os_run_command(cmd,ugid,16)) {
			ERR("id -u command failed\n");
			options.uid = getuid(); // current user id 
		} else
			options.uid = atoi(ugid);

		// group ID 
		memset(ugid, 0, 16);
		memset(cmd, 0, 64);
		sprintf(cmd,"id -g %s",options.user);
		if (os_run_command(cmd,ugid,16)) {
			ERR("id -g command failed\n");
			options.gid = getgid(); // current group id 
		} else
			options.gid = atoi(ugid);
	} while(0);

	// making a session for fuse 
	lpps.session = fuse_get_session( lpps.fuse );
	if (lpps.session == NULL) {
		ERR("fuse_session_new() failed\n");
		exit(1);
	}

	ret = fuse_set_signal_handlers( lpps.session , (fuse_sigfunc_t *)lpps_signal_handler );
	if (ret != 0) {
		fuse_destroy( lpps.fuse );
		exit(1);
	}

	// file handle setup
	if ( prepare_fh( options.mshare ) < 0 ) {
		DBG("File handle setup error \n");
		goto EXIT_lpps_main;
	}

	// mounting a fuse file system
	fuse_mount( lpps.fuse, lpps.mntpt );
    
	ret = fcntl(fuse_session_fd(lpps.session), F_SETFD, FD_CLOEXEC);
	if (ret == -1) {
		ERR("fcntl(F_SETFD,FD_CLOEXEC) failed\n");
		goto EXIT_lpps_main;
	}

	// network multicast 
	if (options.network) 
		lpps_multicast_setup();

	// daemonizing fuse instance 
	fuse_daemonize( !sys_daemon );

	// create / directory 
	do {
		lpps_inode_t * tmp = create_inode( "/" );
		if (!tmp) {
			ERR("create_inode( / )::failed\n");
			goto EXIT_lpps_main;
		}

		tmp->type = LPPS_DIR;
		tmp->mode = S_IFDIR | 0775;
		tmp->nlink = 2;

		lpps_list_push( &(lpps.root) , tmp );
	} while(0); 

	// fuse infinite loop - multithread
	ret = fuse_loop_mt(lpps.fuse, 0); 

EXIT_lpps_main:
	DBG("\n FUSE DONE....\n");

	// cleaning up lpps 
	lpps_list_clean( &(lpps.root) );

	fuse_remove_signal_handlers(lpps.session);
	fuse_unmount(lpps.fuse);
	fuse_destroy(lpps.fuse);
	fuse_opt_free_args(&args);

	free( lpps.mntpt );

	os_rwlock_delete( lpps.mut );

	if (use_syslog)
		closelog();

	//
	// TODO. Configuration value (conf_t) freeing????
	//

	return 0;
}

