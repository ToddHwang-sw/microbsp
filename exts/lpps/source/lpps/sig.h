#ifndef __SIGNAL_HEADER__
#define __SIGNAL_HEADER__

typedef struct {
	char name[16];

	#define OS_LOCK_LOCK 0x01
	#define OS_LOCK_WAIT 0x02
	
	#define OS_LOCK_STATE_DONE	0x80
	#define OS_LOCK_STATE_WRITE	0x40
	#define OS_LOCK_STATE_INTR	0x20
	int state;

	pthread_t owner;
	pthread_mutex_t mut;
	pthread_cond_t  cond;

	/* event locking ... */
	pthread_rwlock_t evlock;

}__attribute__((aligned)) os_sig_t;
#define signal_waiting(sig)	bit_includes((sig)->state,OS_LOCK_WAIT)

/*
 * Reader/Writer lock ...
 */
#define os_rwlock_t	pthread_rwlock_t
#define os_rlock(m)	pthread_rwlock_rdlock( m )
#define os_wlock(m)	pthread_rwlock_wrlock( m )
#define os_rwunlock(m)	pthread_rwlock_unlock( m )
extern os_rwlock_t  * os_rwlock_init();
extern void os_rwlock_delete( os_rwlock_t * mut );

//
// nanoseconds basis ...  please refer to pthread_cond_timedwait(). 
//
#define OS_USEC(n)		((n)*1000)
#define OS_MSEC(n)		(OS_USEC(n)*1000)
#define OS_SEC(n)		(OS_MSEC(n)*1000)

extern os_sig_t * os_sig_init( const char * name );
extern int os_sig_lock( os_sig_t * sig );
extern int os_sig_trylock( os_sig_t * sig );
extern int os_sig_unlock( os_sig_t * sig );
extern int os_sig_wait( os_sig_t * sig, int tout );
extern int os_sig_wakeup( os_sig_t * sig , int uflag );
extern void os_sig_delete( os_sig_t * sig );
extern int os_sig_onwait( os_sig_t *sig );

typedef struct {
	os_rwlock_t lock;
	os_sig_t *sig;
	int val;
	#define EVT_ALLBIT ((int)(~0x0))
}__attribute__((aligned)) os_evt_t;

extern os_evt_t * os_event_init( char *name );
extern int os_event_receive( os_evt_t * evt , int val , int maxcnt /* 10usec */ );
extern int os_event_send( os_evt_t *evt , int val );
extern int os_event_delete( os_evt_t *evt );

// command execution 
extern int os_run_command( char *cmd, char *res, int sz );

#endif /* __SIGNAL_HEADER__ */
