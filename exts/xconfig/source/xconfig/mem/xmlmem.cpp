#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "malloc.h"
#include "xmlmem.h"
#include "common.h"

static struct memspace *xml_memspc = NULL;
static pthread_mutex_t xml_memlock;

#define _xmlmem_init    pthread_mutex_init(&xml_memlock,NULL);
#define _xmlmem_lock    pthread_mutex_lock(&xml_memlock);
#define _xmlmem_unlock  pthread_mutex_unlock(&xml_memlock);

/*
 * I N T E R F A C E S    F O R    D E B U G G I N G
 */
static __inline__ void __create_mem_block(size_t size)
{
	if (xml_memspc)
		return;
	_xmlmem_init
	    xml_memspc = mem_init( size );
	return;
}

void *  _os_malloc(char *f,int l, size_t size)
{
	void *p;
	__create_mem_block( sys_rmem /*XML_MEMSZ*/ );
	_xmlmem_lock
	    p = mem_alloc(xml_memspc, size);
	MEMDBG("%-20s:%-5d - MALLOC*( size=%-7d %5d ) %p\n",f,l,(int)mem_size(p),(int)spc_size(xml_memspc),p);
	_xmlmem_unlock
	return p;
}

void *  _os_calloc(char *f,int l,size_t nmemb, size_t size)
{
	void *p;
	__create_mem_block( sys_rmem /*XML_MEMSZ*/ );
	_xmlmem_lock
	    p = mem_alloc(xml_memspc, (size) * (nmemb));
	MEMDBG("%-20s:%-5d - CALLOC*( size=%-7d %5d ) %p\n",f,l,(int)mem_size(p),(int)spc_size(xml_memspc),p);
	_xmlmem_unlock
	return p;
}

void *  _os_realloc(char *f,int l,void *ptr, size_t size)
{
	void *p;
	__create_mem_block( sys_rmem /*XML_MEMSZ*/ );
	_xmlmem_lock
	    p = mem_realloc(xml_memspc, ptr, size);
	MEMDBG("%-20s:%-5d - REALLOC( size=%-7d %5d ) %p\n",f,l,(int)mem_size(p),(int)spc_size(xml_memspc),p);
	_xmlmem_unlock
	return p;
}

char *  _os_strdup(char *f,int l,const char *s)
{
	char *p;
	__create_mem_block( sys_rmem /*XML_MEMSZ*/ );
	_xmlmem_lock
	    p = (char *)mem_strdup(xml_memspc, s);
	MEMDBG("%-20s:%-5d - STRDUP*( size=%-7d %5d ) %p\n",f,l,(int)mem_size(p),(int)spc_size(xml_memspc),p);
	_xmlmem_unlock
	return p;
}

void    _os_free(char *f,int l,void *ptr)
{
	unsigned int sz;

	if (!ptr)
		return;

	_xmlmem_lock
	    sz = mem_size(ptr);
	mem_free(xml_memspc, ptr);
	MEMDBG("%-20s:%-5d - FREE***( size=%-7d %5d ) %p\n",f,l,sz,(int)spc_size(xml_memspc),ptr);
	_xmlmem_unlock
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
	return (int)spc_size(xml_memspc);
}

