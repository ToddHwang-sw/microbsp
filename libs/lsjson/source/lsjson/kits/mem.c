#include "common.h"

static struct memspace *_memspc = NULL;
static pthread_mutex_t _memlock;

#define _mem_init    pthread_mutex_init(&_memlock,NULL);
#define _mem_lock    pthread_mutex_lock(&_memlock);
#define _mem_unlock  pthread_mutex_unlock(&_memlock);

int mem_dbg = 0;
#define memdbg(fmt,args...)	if(mem_dbg) printf(fmt,##args)

/* memory align for architecture-dependent memory access variation */
#define aligned_size(sz)	(((sz)+16) & ~15)

/*
 *
 * I N T E R F A C E S    F O R    D E B U G G I N G
 *
 */
void __os_meminit(size_t size)
{
	if (_memspc)
		return;

	_mem_init
	_memspc = mem_init( aligned_size(size) );

	return;
}

void *  _os_malloc(char *f,int l, size_t size)
{
	void *p;

	__os_meminit( sys_rmem );
	_mem_lock
	p = mem_alloc(_memspc, aligned_size(size));
	memdbg("%p %-20s:%-5d - MALLOC*( size=%-7d %5d )\n",p, f,l,(int)mem_size(p),(int)spc_size(_memspc));
#ifdef MEMORY_TRACK
	{
		struct memheader *mh = (struct memheader *)(p - sizeof(struct memheader));
		memset(mh->fn,0,32);
		strncpy(mh->fn,f,32-1);
		mh->line = l;
	}
#endif
	_mem_unlock

	return p;
}

void *  _os_calloc(char *f,int l,size_t nmemb, size_t size)
{
	void *p;

	__os_meminit( sys_rmem );
	_mem_lock
	p = mem_alloc(_memspc, aligned_size(size) * (nmemb));
	memdbg("%p %-20s:%-5d - CALLOC*( size=%-7d %5d )\n",p, f,l,(int)mem_size(p),(int)spc_size(_memspc));
#ifdef MEMORY_TRACK
	{
		struct memheader *mh = (struct memheader *)(p - sizeof(struct memheader));
		memset(mh->fn,0,32);
		strncpy(mh->fn,f,32-1);
		mh->line = l;
	}
#endif
	_mem_unlock

	return p;
}

void *  _os_realloc(char *f,int l,void *ptr, size_t size)
{
	void *p;

	__os_meminit( sys_rmem );
	_mem_lock
	p = mem_realloc(_memspc, ptr, aligned_size(size));
	memdbg("%p %-20s:%-5d - REALLOC( size=%-7d %5d )\n",p, f,l,(int)mem_size(p),(int)spc_size(_memspc));
#ifdef MEMORY_TRACK
	{
		struct memheader *mh = (struct memheader *)(p - sizeof(struct memheader));
		memset(mh->fn,0,32);
		strncpy(mh->fn,f,32-1);
		mh->line = l;
	}
#endif
	_mem_unlock

	return p;
}

char *  _os_strdup(char *f,int l,const char *s)
{
	char *p;

	__os_meminit( sys_rmem );
	_mem_lock
	p = (char *)mem_strdup(_memspc, s);
	memdbg("%p %-20s:%-5d - STRDUP*( size=%-7d %5d )\n",p, f,l,(int)mem_size(p),(int)spc_size(_memspc));
#ifdef MEMORY_TRACK
	{
		struct memheader *mh = (struct memheader *)(p - sizeof(struct memheader));
		memset(mh->fn,0,32);
		strncpy(mh->fn,f,32-1);
		mh->line = l;
	}
#endif
	_mem_unlock

	return p;
}

void    _os_free(char *f,int l,void *ptr)
{
	unsigned int sz;

	if (!ptr)
		return;

	_mem_lock
	sz = mem_size(ptr);
	mem_free(_memspc, ptr);
	memdbg("%p %-20s:%-5d - FREE***( size=%-7d %5d )\n",ptr, f,l,sz,(int)spc_size(_memspc));
	_mem_unlock
}

/*
 * I N T E R F A C E S    F O R    N O N - D E B U G G I N G
 */
void *  __os_malloc(size_t size)
{
	return _os_malloc(NULL,0,size);
}

void *  __os_calloc(size_t nmemb, size_t size)
{
	return _os_calloc(NULL,0,nmemb,size);
}

void *  __os_realloc(void *ptr, size_t size)
{
	return _os_realloc(NULL,0,ptr,size);
}

char *  __os_strdup(const char *s)
{
	return _os_strdup(NULL,0,s);
}

void    __os_free(void *ptr)
{
	_os_free(NULL,0,ptr);
}

int     __os_memspc( void )
{
	return (int)spc_size(_memspc);
}

int     __os_memblks( void (*func)(void *) )
{
	return (int)mem_navigate(_memspc,func);
}

