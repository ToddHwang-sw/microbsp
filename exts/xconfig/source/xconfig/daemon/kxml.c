#ifdef __KERNEL__
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/version.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif 
#include "kxml.h"

#ifdef __KERNEL__
	#define printX(fmt,args...)	printk(fmt,##args)
	#define mallocX(sz)		kmalloc(sz,GFP_KERNEL)
	#define freeX(p)		kfree(p)
#else
	#define printX(fmt,args...)	printf(fmt,##args)
	#define mallocX(sz)		malloc(sz)
	#define freeX(p)		free(p)
#endif 

static int verbose = 0;
#define PRINT(fmt,args...)	if (verbose){ printX(fmt,##args); }

/********************************************
 *
 *
 * P R I V A T E   I N T E R F A C E 
 *
 * 
 */
static __inline__ int skippable_char( char c )
{
	if ( 
		((c >= '0') && (c <= '9')) ||
		((c >= 'a') && (c <= 'z')) ||
		((c >= 'A') && (c <= 'Z')) ||
		(c == '>') ||
		(c == '<') ||
		(c == '?') ||
		(c == '/') ||
		(0)
	) 
		return 0; 
	else 
		return 1;
}

/* adding path */
static void append_tag( char *root , char *node )
{
	if (!root || !node) 
		return ;

	strcat(root , "/" );
	strcat(root , node );
}

/* subtracting path */
static void shrink_tag( char *root )
{
	char *s,*p;

	if (!root) 
		return ;

#ifndef __KERNEL__
	p = rindex( root , '/' );
#else 
	s = root;
	p = (char *)&(root[ strlen(root)-1 ]); 
	while ( (*p != '/') && (p != s) ) --p;
	if (p == s) {
		// end of parse...
		return ;
	}
#endif 
	*p = 0x0;  /* truncate */
}

#define MAX_MEM_PAGES 	16
#define MEM_PAGE_SIZE 	(4*1024)
#define OVRSZ		32
static int page_cnt = -1;
static char * page_root[MAX_MEM_PAGES] = {NULL,}; /* prevent fragments */
static char * page_loc = NULL;
static int page_alloced = 0;

static char * alloc_xml_page( int * cnt )
{
	char *data;

	/* Check... */
	if ((*cnt) > 0) {
		if(((*cnt)+1) >= MAX_MEM_PAGES) {
			printX("XML page full\n");
			return NULL;
		}
	}

	data = (char *)mallocX( MEM_PAGE_SIZE );
	if (!data)
		return NULL;

	/* Resetting counter */
	if ( (*cnt) == (-1) ) 
		(*cnt) = 0;
	else 
		++ (*cnt);

	return data;
}

static char * XmlMalloc( int sz )
{
	int amount;
	char *p;

	if (sz == 0)
		return NULL;

	amount = (sz & ~0x3) + 4; // 4 bytes align 

	if ((page_cnt == (-1)) ||
			((page_alloced+amount) >= (MEM_PAGE_SIZE-OVRSZ)) ) {
		page_loc = alloc_xml_page(&page_cnt);	
		if (!page_loc) {
			printX("alloc_xml_page::failed\n");
			return NULL;
		}
		page_alloced = 0;
		page_root[ page_cnt ] = page_loc;
		PRINT("XML MEMORY ALLOCATED %-3d %dbytes \n",page_cnt,MEM_PAGE_SIZE);
	}
		
	p = page_loc;
	page_loc += amount; 
	page_alloced += amount;

	return p;
}

static void XmlFree( void *p )
{
	/* Not Supported */
}

static int add_to_xml( struct XmlData **root , char *sym , char *val )
{
	struct XmlData * new;

	/* new node */
	new = (struct XmlData *)XmlMalloc(sizeof(struct XmlData));
	if (!new) {
		printX("XmlMalloc(sizeof(struct XmlData)):error\n");
		return 1;
	}
	new->next = NULL;

	/* value */
	if (strlen(val) > 0) {
		new->value = XmlMalloc(strlen(val)+4);
		if (!new->value){
			XmlFree(new);
			printX("XmlMalloc(value=strlen(val)):error\n");
			return 1;
		}
		memset( new->value , 0 , strlen(val)+4);
		strcpy( new->value , val );
	}else
		new->value = NULL;

	/* path */
	new->path = XmlMalloc(strlen(sym)+4);
	if (!new->path) {
		if(new->value) XmlFree(new->value);
		XmlFree(new);
		printX("XmlMalloc(path=strlen(sym)):error\n");
		return 1;
	}
	memset( new->path , 0 , strlen(sym)+4);
	strcpy( new->path , sym+strlen(PATH_PREFIX) );


	/* Append... */
	if (!(*root)) {
		*root = new;
	} else {
		struct XmlData * p = *root;

		while( p->next ) p = p->next;
		p->next = new;
	}

	return 0;	
}

/********************************************
 *
 *
 * P U B L I C    I N T E R F A C E 
 *
 * 
 */
#define NEXT_CH		{p++;}
#define SKIP_WHITE	do{ while( skippable_char( *p ) ) NEXT_CH }while(0);
#define GET_CH		*p
#define AHEAD_CH(x)	*(p+(x))

/* XML header parsing */
char * xml_header( char *start ) 
{
	char *p = start;
	static char hsym[ 2048 ];
	static int hsymp; /* header symbol */

	hsymp = 0;
	memset( hsym , 0 , 2048 );

	SKIP_WHITE
	if (GET_CH == '<') {
		NEXT_CH
		if (GET_CH != '?') {
			printX("%s(ERROR::A)\n",__FUNCTION__);
			return NULL;
		}
		do{
			hsym[hsymp++] = GET_CH;
			NEXT_CH
		}while(GET_CH != '?');
		NEXT_CH	
		if (GET_CH != '>') {
			printX("%s(ERROR::B)\n",__FUNCTION__);
			return NULL;
		}	
		NEXT_CH
	}
	printX("XML HEADER SYMBOL (%s) \n",hsym);
	return p;
}

#define SYM_LEN 64
static char root_tag[ SYM_LEN*4 ];

/* XML body parsing */
struct XmlData *  
xml_parse( char *data , int verb )
{
	int depth = 0;
	static char symbol[ SYM_LEN ];
	static int symp; /* header symbol */
	#define VAL_LEN 2048
	static char value[ VAL_LEN ];
	static int valp; /* header symbol */
	char *p = (char *)data;
	struct XmlData *xml = NULL;

	verbose = verb;

	memset( root_tag, 0, SYM_LEN*4 );

	do{
		SKIP_WHITE
		if (GET_CH == '<') {
			NEXT_CH
			if (GET_CH == '!') {
				/* Comments */
				int cdone = 0; /* comments done flag */
				while( !cdone ){
					if ((GET_CH == '-') &&
							(AHEAD_CH(1) == '-') &&
							(AHEAD_CH(2) == '>') ) {
						NEXT_CH
						cdone = 1;
					}
					NEXT_CH
				}
				NEXT_CH
			} else if (GET_CH == '/') {
				/* Terminating Symbol */
				while( GET_CH != '>' ) NEXT_CH
				NEXT_CH
				--depth;
				shrink_tag( root_tag ); 
				//printX("[%d]TSymbol=(%s)\n",depth,symbol);
			} else {
				/* next symbol */
				int cform = 0; /* condensed form */
				memset(symbol,0,SYM_LEN); symp = 0;
				while( GET_CH != '>' ) {
					if ( (AHEAD_CH(1) == '/') &&
							(AHEAD_CH(2) == '>') ) {
						/* Condensed form */
						memset(value,0,VAL_LEN); valp = 0; /* empty value */
						cform = 1;
					}
					symbol[ symp++ ] = GET_CH;
					NEXT_CH
					if (cform) {
						NEXT_CH
						append_tag( root_tag , symbol );
						PRINT("[%-3d] %s::NULL\n",depth,root_tag);
						add_to_xml( &xml , root_tag , value );
						shrink_tag( root_tag ); 
					}
				}
				NEXT_CH
				if ( GET_CH == '<' ) {
					/* No Value */
					memset(value,0,VAL_LEN); valp = 0;
					do{ 
						NEXT_CH
					}while( GET_CH != '>' );
					NEXT_CH
					append_tag( root_tag , symbol );
					PRINT("[%-3d] %s::NULL\n",depth,root_tag);
					add_to_xml( &xml , root_tag , value );
					shrink_tag( root_tag ); 
				} else {
					SKIP_WHITE /* skip white spaces */
					if (!cform) {
						++depth;
						if ( GET_CH == '<' ) {
							/* Next Block */
							append_tag( root_tag , symbol );
							//printX("[%d]SSymbol=(%s)\n",depth,root_tag);
						} else {
							memset(value,0,VAL_LEN); valp = 0;
							do{
								value[ valp++ ] = GET_CH;
								NEXT_CH
							}while( GET_CH != '<' );
							append_tag( root_tag , symbol );
							PRINT("[%-3d] %s::%s\n",depth,root_tag,value);
							add_to_xml( &xml , root_tag , value );
						}
					} /* if (!cform) */
				} /* if ( GET_CH == '<' ) */
			}	
		}
	}while( depth ); 

	PRINT("Done\n");
	return xml;
}

void xml_print(struct XmlData *xml)
{
	struct XmlData *p = xml;
	int num = 0;
	char *val;

	while( p ) {
		val = p->value ? p->value : "NULL";
		printX("[%-3d] %-32s %32s\n",num++, p->path,val);
		p = p->next;
	}
}

void xml_free(struct XmlData *xml)
{
	struct XmlData *q, *p;
	int ix;

	p = q = xml;
	while( p ) {
		if( p->next ) 
			q = p->next;
		else 
			q = NULL;
		XmlFree( p );
		p = q;
	}

	if (page_cnt == -1)
		return;

	/* Free up blocks */
	for(ix = 0;ix < page_cnt;ix++) 
		if (page_root[ix]) freeX(page_root[ix]);
		
}

#ifndef __KERNEL__

/*
 * To compile, 
 *
 * # gcc -o kxml kxml.c 
 * # ./kxml 
 *
 */
int main()
{
	char *buff;
	char *pblk;
	int fd;
	struct stat st;
	struct XmlData *data;
	
	fd = open("/var/tmp/config.2.xml",O_RDWR);
	if (fd<0) {
		printf("open(...config.2.xml)::failed\n");
		return 1;
	}

	if (fstat(fd, &st) < 0) {
		printf("fstat()::failed\n");
		close(fd);
		return 1;
	}

	printf("File size: %d bytes\n",st.st_size);

	buff = (char *)malloc(st.st_size);
	if( !buff ){
		printf("malloc()::failed\n");
		return 1;
	}
	memset(buff,0,st.st_size);
	read(fd,buff,st.st_size);
	close(fd);

	pblk = xml_header(buff);
	if (!pblk) {
		printf("XML HEADER not found\n");
		free(buff);
		return 1;
	}

	data = xml_parse( pblk , 0 );
	if( data ){
		xml_print(data);
		xml_free(data);
	}

	free(buff);

	return 0;
}
#endif

