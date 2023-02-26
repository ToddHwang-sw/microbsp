#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "os.h"
#include "sig.h"

//
// C++
//
using namespace std;

//
//
//  P U B L I C    I N T E R F A C E S 
//
//
//
os_sig_t * os_sig_init( const char * name )
{
	os_sig_t *temp;

	temp = (os_sig_t *)os_malloc( sizeof( os_sig_t ) );
	if (!temp) {
		ERR(" %s - os_malloc( sizoef( os_sig_t ) )\n",name);
		return NULL;
	}
	memset((char *)temp,0,sizeof(os_sig_t));

	strncpy(temp->name, name?name:"unknown", 15);
	temp->state = 0;
	temp->owner = 0;

	pthread_mutex_init( &(temp->mut) , NULL );
	pthread_cond_init( &(temp->cond), NULL );

	return temp;
}

os_rwlock_t * os_rwlock_init( void )
{
	os_rwlock_t *temp;

	temp = (os_rwlock_t *)os_malloc( sizeof( os_rwlock_t ) );
	if (!temp) {
		ERR(" os_malloc( sizoef( os_rwlock_t ) )\n");
		return NULL;
	}
	pthread_rwlock_init( temp , NULL );

	return temp;
}

void os_rwlock_delete( os_rwlock_t * mut )
{
	os_free( mut );	// may cause memory fault...
}

int os_sig_lock( os_sig_t * sig )
{
	pthread_mutex_lock( &(sig->mut) );
	
	sig->state |= OS_LOCK_LOCK;

	return 0;
}

int os_sig_trylock( os_sig_t * sig )
{
	int ret = pthread_mutex_trylock( &(sig->mut) );

	if (ret == 0) {	
		sig->state |= OS_LOCK_LOCK;
		return 0;
	} else { 
		return -1;
	}
}

int os_sig_unlock( os_sig_t * sig )
{
	sig->state &= ~OS_LOCK_LOCK;

	pthread_mutex_unlock( &(sig->mut) );
	
	return 0;
}

int os_sig_onwait( os_sig_t *sig )
{
	int ret;

	pthread_mutex_lock( &sig->mut );
	ret = sig->state;
	pthread_mutex_unlock( &sig->mut );

	return (ret & OS_LOCK_WAIT) ? 1 : 0;
}

int os_sig_wait( os_sig_t * sig, int tout )
{
	int res;

	if (os_sig_lock( sig )) 
		return -1; 

	sig->state |= OS_LOCK_WAIT;

	/* current thread */
	sig->owner = pthread_self();

	if (tout) {
		struct timespec maxwait;

		maxwait.tv_sec  = (tout / OS_SEC(1)) ; /* second */
		maxwait.tv_nsec = (tout % OS_SEC(1)) ; /* nsec */

		res = pthread_cond_timedwait( &(sig->cond), &(sig->mut), &maxwait );
	} else
		res = pthread_cond_wait( &(sig->cond), &(sig->mut) );	

	/* current thread */
	sig->owner = 0; /* released ~ */
	
	return res;
}

int os_sig_wakeup( os_sig_t * sig , int uflag )
{
	pthread_cond_signal( &(sig->cond) );

	sig->state &= ~OS_LOCK_WAIT;

	//
	// uflag indicates that we succeeded in obtaining a lock
	//
	if (!uflag)
		os_sig_unlock( sig );

	return 0;
}

void os_sig_delete( os_sig_t * sig )
{
	int ret;

	if (!sig)
		return ;

	if (signal_waiting(sig)) {
		//
		// Though lock is not obained, event will be signalled !!
		//
		ret = os_sig_trylock( sig );
		//if (ret)
		//	ERR("SIGNAL LOCK FAILED\n");
		sig->state |= OS_LOCK_STATE_DONE; // ???
		os_sig_wakeup( sig , ret );       // releasing...

		DBG(" DELETE SIG ( %s )\n",sig->name);
		// TODO
		// Just giving a process to arrange resources before os_free()..
		usleep(300*1000);
	}

	pthread_mutex_destroy( &(sig->mut) );
	pthread_cond_destroy( &(sig->cond) );

	os_free( sig );	// may cause memory fault...
}

//
//
// Eventing... synchronize..
//
//
os_evt_t * os_event_init( char *name )
{
	os_evt_t *temp;

	temp = (os_evt_t *)os_malloc( sizeof(os_evt_t) );
	if (!temp)
		return NULL;

	temp->sig = os_sig_init( name );
	if (!temp->sig) {
		os_free( temp );
		return NULL;
	}

	temp->val = 0;

	return temp;
}

//
// maxcnt <= 0 -- endless loop 
// maxcnt > 0  -- 1000usec X maxcnt times looping 
//
int os_event_receive( os_evt_t * evt , int val , int maxcnt )
{
	int ret = 0;
	int cnt = 0;
	int _mxcnt = (maxcnt < 0) ? 1 : maxcnt;
	
	while ( !bit_includes(evt->val,val) && (cnt <= _mxcnt)) {
		if (maxcnt > 0) 
			++cnt;
		ret = os_sig_wait( evt->sig, OS_USEC(100) );
		if (ret != ETIMEDOUT)
			break;

		os_sig_unlock( evt->sig );
	}

	return evt->val;
}

int os_event_send( os_evt_t *evt , int val ) 
{
	int ret;

	if (!evt)
		return -1;

	//
	// Though lock is not obained, event will be signalled !!
	//
	ret = os_sig_trylock( evt->sig );
	//
	// Too noisy...
	//
	//if (ret)
	//	ERR("EVENT LOCK FAILED\n");
	bit_adds( evt->val, val );
	os_sig_wakeup( evt->sig , ret );
	return 0;
}

int os_event_delete( os_evt_t *evt ) 
{
	if (!evt)
		return -1;

	os_sig_delete( evt->sig );
	os_free( evt );

	return 0;
}

int os_run_command( char *cmd, char *res, int sz )
{
	FILE * fp;
	int ret = 0;

	if (!res || !sz)
		return -1;

	fp = popen(cmd, "r");
	if (fp == NULL)
		return -1; /* error */

	if (fgets(res,sz-1,fp) == NULL) {
		ret = -1;
		goto exit_OS_RUN_COMMAND;
	}

	/* good .. */

exit_OS_RUN_COMMAND:
	pclose(fp);
	return ret;
}

