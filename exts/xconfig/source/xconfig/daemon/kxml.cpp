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
#ifdef XCONFIG_PATCH
	#include	"malloc.h"
	#include	"xmlmem.h"

#ifdef STANDALONE
	int sys_rmem = 4*1024*1024;
	int sys_loglevel = 2;
#endif
#endif
#include "kxml.h"

#ifdef __KERNEL__
	#define printX(fmt,args...)	printk(fmt,##args)
	#define mallocX(sz)		kmalloc(sz,GFP_KERNEL)
	#define freeX(p)		kfree(p)
#else
	#define printX(fmt,args...)	printf(fmt,##args)
#ifdef XCONFIG_PATCH
	#define mallocX(sz)		__os_malloc(sz)
	#define freeX(p)		__os_free(p)
#else
	#define mallocX(sz)		malloc(sz)
	#define freeX(p)		free(p)
#endif 
#endif 

static int hlen = 0; /* header length */
static char *ploc;
static char *xmlstart = NULL;
static int xmlsize = 0;
static int verbose = 1;
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
static void append_tag( int offset, char *root , char *node )
{
	if (!root || !node) 
		return ;

	strcat(root , "/" );
	strcat(root , node );
}

/* subtracting path */
static void shrink_tag( char *root )
{
#ifdef __KERNEL__
	char *s;
#endif
	char *p;

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

static int add_to_xml( struct XmlData **root , int loc , char *sym , char *val )
{
	struct XmlData * node;

	/* node node */
	node = (struct XmlData *)XmlMalloc(sizeof(struct XmlData));
	if (!node) {
		printX("XmlMalloc(sizeof(struct XmlData)):error\n");
		return 1;
	}
	node->next = NULL;

	/* value */
	if (strlen(val) > 0) {
		node->value = XmlMalloc(strlen(val)+4);
		if (!node->value){
			XmlFree(node);
			printX("XmlMalloc(value=strlen(val)):error\n");
			return 1;
		}
		memset( node->value , 0 , strlen(val)+4);
		strcpy( node->value , val );
	}else
		node->value = NULL;

	/* path */
	node->path = XmlMalloc(strlen(sym)+4);
	if (!node->path) {
		if(node->value) XmlFree(node->value);
		XmlFree(node);
		printX("XmlMalloc(path=strlen(sym)):error\n");
		return 1;
	}
	memset( node->path , 0 , strlen(sym)+4);
	strcpy( node->path , sym+strlen(PATH_PREFIX) );

	{
		char *lsym;
		if ((lsym=strrchr(node->path,'/')))
			lsym = lsym + 1; /* Skipping first '/' */
		else
			lsym = node->path;
		node->loc = loc - (strlen(lsym) + strlen(val) + 2); /* 2 -> '<' .. '>' */
		node->newval = NULL;
	}

	/* Append... */
	if (!(*root)) {
		*root = node;
	} else {
		struct XmlData * p = *root;

		while( p->next ) p = p->next;
		p->next = node;
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
#if 0
#define NEXT_CH		{p++;}
#define SKIP_WHITE	do{ while( skippable_char( *p ) ) NEXT_CH }while(0);
#define GET_CH		*p
#define AHEAD_CH(x)	*(p+(x))
#endif
#define NEXT_CH		{ploc++;}
#define SKIP_WHITE	do{ while( skippable_char( *ploc ) ) NEXT_CH }while(0);
#define GET_CH		*ploc
#define AHEAD_CH(x)	*(ploc+(x))

/* XML header parsing */
char * kxml_header( char *start ) 
{
	/* char *p = start; */
	static char hsym[ 2048 ];
	static int hsymp; /* header symbol */

	ploc = xmlstart = start;
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
	/* hlen = p - start; */
	hlen = ploc - start;
	printX("XML HEADER SYMBOL (%s) SIZE (%d) \n",hsym, hlen);
	return ploc;
}

#define SYM_LEN 64
static char root_tag[ SYM_LEN*4 ];

/* XML body parsing */
struct XmlData *  
kxml_parse( char *data , int verb )
{
	int depth = 0;
	static char symbol[ SYM_LEN ];
	static int symp; /* header symbol */
	#define VAL_LEN 2048
	static char value[ VAL_LEN ];
	static int valp; /* header symbol */
	/* char *p = (char *)data; */
	struct XmlData *xml = NULL;

	ploc = (char *)data;
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
						append_tag( (int)(ploc - xmlstart) , root_tag , symbol );
						PRINT("[%-3d] %s::NULL\n",depth,root_tag);
						add_to_xml( &xml , (int)(ploc - xmlstart) , root_tag , value );
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
					append_tag( (int)(ploc - xmlstart) , root_tag , symbol );
					PRINT("[%-3d] %s::NULL\n",depth,root_tag);
					add_to_xml( &xml , (int)(ploc - xmlstart) , root_tag , value );
					shrink_tag( root_tag ); 
				} else {
					SKIP_WHITE /* skip white spaces */
					if (!cform) {
						++depth;
						if ( GET_CH == '<' ) {
							/* Next Block */
							append_tag( (int)(ploc - xmlstart), root_tag , symbol );
							//printX("[%d]SSymbol=(%s)\n",depth,root_tag);
						} else {
							memset(value,0,VAL_LEN); valp = 0;
							do{
								value[ valp++ ] = GET_CH;
								NEXT_CH
							}while( GET_CH != '<' );
							append_tag( (int)(ploc - xmlstart), root_tag , symbol );
							PRINT("[%-3d] %s::%s\n",depth,root_tag,value);
							add_to_xml( &xml , (int)(ploc - xmlstart) , root_tag , value );
						}
					} /* if (!cform) */
				} /* if ( GET_CH == '<' ) */
			}	
		}
	}while( depth ); 

	xmlsize = (int)(ploc - xmlstart);
	PRINT("Done (%d)\n",xmlsize);
	return xml;
}

	/* set */
int kxml_set(struct XmlData *xml, char *path, char *newval)
{
	struct XmlData *p = xml;

	if (!path || !newval)
		return 0;

	/* XML buffer should exist .. */
	if (!xmlstart)
		return 0;

	while( p ) {
		if (strcmp(p->path,path)) {
			p = p->next;
			continue;
		}
		PRINT("%s:: found !!\n", __func__);
		if (p->newval) 
			XmlFree(p->newval);
		p->newval = XmlMalloc(strlen(newval)+4);
		if (!p->newval) {
			PRINT("%s:: XmlMalloc( %d ) failed\n",__func__,(int)strlen(newval)+4);
			return -1;
		}
		memset(p->newval, 0, strlen(newval)+4);
		memcpy(p->newval, newval, strlen(newval) );
		PRINT("%s: change value %s (%s:%d) -> (%s:%d) \n", __func__,
				p->path, p->value, (int)strlen(p->value), p->newval, (int)strlen(p->newval));
		break;
	}

	return 0;
}

int kxml_write(struct XmlData *xml, char *nmem, int nsize )
{
	struct XmlData *p = xml;
	int nmemp = 0;
	char *buff;
	int xlen = 0;
	char *path;

	if (!xml)
		return 0;

	/* XML buffer should exist .. */
	if (!xmlstart)
		return 0;

	if (!nmem)
		return 0;

	/* init ... */
	memset( nmem, 0, nsize );

	while( p ) {
		if (!p->newval)  {
			p = p->next;
			continue;
		}

		/* First block writing ...*/
		memcpy(nmem + nmemp, xmlstart, p->loc ); 
		nmemp += p->loc;

		/* writing new value */
		xlen = (strlen(p->path) * 2) + strlen(p->newval) + 16;
		buff = (char *)XmlMalloc( xlen );
		if (!buff)
			return -1;
		memset(buff,0,xlen);

		if ((path=strrchr(p->path,'/')))
			path = path + 1; /* Skipping first '/' */
		else
			path = p->path;

		xlen = sprintf(buff,"<%s>%s</%s>",path,p->newval,path);
		memcpy(nmem + nmemp, buff, xlen); /* XLEN */
		nmemp += xlen;
		XmlFree( buff );

		xlen = strlen(path) * 2 + strlen(p->value) +
				( 1 + 1 ) * 2 + /* ( '<' + '>' ) x 2 */
				1  /* '\' */
				;
		xlen += p->loc;

		memcpy(nmem + nmemp, xmlstart + xlen , xmlsize - xlen ); /* Final Block */

		return nmemp + (xmlsize - xlen);
	}

	return 0;
}

void kxml_print(struct XmlData *xml)
{
	struct XmlData *p = xml;
	int num = 0;
	char *val;

	while( p ) {
		val = (char *)( p->value ? p->value : "NULL" );
		printX("[%-3d] %-5d %-32s %32s | ",num++, p->loc, p->path,val);
		printX("%c %c %c %c %c %c %c %c ",
				*(xmlstart + p->loc),
				*(xmlstart + p->loc + 1),
				*(xmlstart + p->loc + 2),
				*(xmlstart + p->loc + 3),
				*(xmlstart + p->loc + 4),
				*(xmlstart + p->loc + 5),
				*(xmlstart + p->loc + 6),
				*(xmlstart + p->loc + 7) );
		printX("\n");
		p = p->next;
	}
}

void kxml_free(struct XmlData *xml)
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
#ifdef STANDALONE
/*
 * To compile, 
 *
 * # gcc -o kxml kxml.c 
 * # ./kxml 
 *
 */
int main()
{
	char *buff[2];
	char *fn[2];
	int fd[2];
	char *pblk;
	int i;
	int  active = 0;
	int  nactive = 0;
	int nsize;
	struct stat st;
	struct XmlData *data;
	
	/* buffer allocation */
	for( i = 0 ; i < 2; i++ ) {
		buff[ i ] = (char *)malloc( 1024*1024 );
		if( !buff[ i ] ){
			printf("malloc(buff)::failed\n");
			return 1;
		}
		memset(buff[ i ],0, 1024*1024 );
	}

	active = 0;

	fn[0] = (char *)"/var/tmp/config.2.xml";
	fn[1] = (char *)"/var/tmp/config.3.xml";

	if ( 1 ) {

		fd[active] = open(fn[active],O_RDWR);
		if (fd[active]<0) {
			printf("open(%s)::failed\n",fn[active]);
			return 1;
		}

		if (fstat(fd[active], &st) < 0) {
			printf("fstat(%s)::failed\n",fn[active]);
			close(fd[active]);
			return 1;
		}

		printf("File size: %d bytes\n",(int)st.st_size);

		read(fd[ active ],buff[ active ],st.st_size);
		close(fd[ active ]);

		pblk = kxml_header(buff[ active ]);
		if (!pblk) {
			printf("XML HEADER not found\n");
			free(buff[0]);
			free(buff[1]);
			return 1;
		}

		data = kxml_parse( pblk , 0 );
		if (!data) {
			printf("XML PARSE ERROR\n");
			free(buff[0]);
			free(buff[1]);
			return 1;
		}

		kxml_print(data);

		/* set */
		kxml_set(data, (char *)"product", (char *)"ABCDEFGHIJK" );

		nactive = active == 0 ? 1 : 0;

		nsize = kxml_write(data, buff[ nactive ], st.st_size + 256 );

		fd[ nactive ] = open(fn[ nactive ],O_RDWR);
		if (fd[ nactive ]<0) {
			printf("open(%s)::failed\n",fn[ nactive ]);
			return 1;
		}

		write(fd[ nactive ],buff[ nactive ], nsize);
		close(fd[ nactive ]);

		kxml_free(data);
	}

	return 0;
}
#endif /* STANDALONE */
#endif /* XCONFIG_PATCH */

