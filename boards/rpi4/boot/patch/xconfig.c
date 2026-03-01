/*
 * Todd Hwang 
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <command.h>
#include <net.h>
#include <malloc.h>
#include <vsprintf.h>
#include <env.h>

/*
 *
 *
 * X M L     C O N F I G U R A T I O N      P A R S E R 
 *
 *
 */
#define PATH_PREFIX	"/config/"

struct XmlData{
	char *path;
	char *value;
	char *newval;
	unsigned int loc;
	struct XmlData *next;
};

#define printX		printf
#define mallocX		malloc
#define freeX		free

static unsigned long pcaddr; /* initial config block address */
static int hlen = 0; /* header length */
static char *ploc;
static char *xmlstart = NULL;
static int xmlsize = 0;
static int verbose = 0;
#define PRINT(fmt,args...)	{if (verbose){ printX(fmt,##args); }}

/* temporal XML tree for image update use */
static char *template_xml = 
	{
		"<?xml version=\"0.0.0.1\" encoding=\"utf-8\"?>\n\r"
		"<config>\n\r"
		"\t<product>Raspberry PI WiFi-WiFi CPE</product>\n\r"
		"\t<date>202304112030</date>\n\r"
		"\t<model>RPICPE</model>\n\r"
		"\t<storage>\n\r"
		"\t\t<kernel>\n\r"
		"\t\t\t<k0>\n\r"
		"\t\t\t\t<active>0</active>\n\r"
		"\t\t\t\t<crc>0</crc>\n\r"
		"\t\t\t\t<size>0</size>\n\r"
		"\t\t\t\t<version>0</version>\n\r"
		"\t\t\t</k0>\n\r"
		"\t\t\t<k1>\n\r"
		"\t\t\t\t<active>0</active>\n\r"
		"\t\t\t\t<crc>0</crc>\n\r"
		"\t\t\t\t<size>0</size>\n\r"
		"\t\t\t\t<version>0</version>\n\r"
		"\t\t\t</k1>\n\r"
		"\t\t</kernel>\n\r"
		"\t</storage>\n\r"
		"</config>"
	};

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

	s = root;
	p = (char *)&(root[ strlen(root)-1 ]); 
	while ( (*p != '/') && (p != s) ) --p;
	if (p == s) {
		// end of parse...
		return ;
	}
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

static int add_to_xml( struct XmlData **root , int loc, char *sym , char *val )
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

	{
		char *lsym;
		if ((lsym=strrchr(new->path,'/')))
			lsym = lsym + 1; /* Skipping first '/' */
		else
			lsym = new->path;
		new->loc = loc - (strlen(lsym) + strlen(val) + 2); /* 2 -> '<' .. '>' */
		new->newval = NULL;
	}

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

/* Update CRC */
static __inline__ unsigned short xml_config_crc( char *data , int size )
{
	int adjsize;
	unsigned int total;
	unsigned short *u16p;
	int ix;

	adjsize = size & ~0x1; /* 16 bit align */

	total = 0;
	u16p = (unsigned short *)data;
	for(ix = 0; ix < (adjsize / (int)sizeof(unsigned short)); ix++ ) 
		total += (unsigned short)(*u16p++); 

	total = (total >> 16) + (total & 0xFFFF);
	total = (total >> 16) + total;
	total = ~total;

	return (unsigned short)total;
}

/********************************************
 *
 *
 * P U B L I C    I N T E R F A C E 
 *
 * 
 */
#define NEXT_CH		{ploc++;}
#define SKIP_WHITE	do{ while( skippable_char( *ploc ) ) NEXT_CH }while(0);
#define GET_CH		*ploc
#define AHEAD_CH(x)	*(ploc+(x))

/* XML header parsing */
static char * xml_header( unsigned long start ) 
{
	/* char *p = (char *)start; */
	static char hsym[ 2048 ];
	static int hsymp; /* header symbol */

	ploc = xmlstart = (char *)start;
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
	/* printX("XML HEADER SYMBOL (%s) \n",hsym); */
	hlen = (int)((unsigned long)ploc - start);
	/* printX("XML HEADER SYMBOL (%s) SIZE (%d) \n",hsym, hlen); */
	return ploc;
}

#define SYM_LEN 64
static char root_tag[ SYM_LEN*4 ];

/* XML body parsing */
static struct XmlData *  
xml_parse( char *data , int verb )
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
						append_tag( root_tag , symbol );
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
					append_tag( root_tag , symbol );
					PRINT("[%-3d] %s::NULL\n",depth,root_tag);
					add_to_xml( &xml , (int)(ploc - xmlstart) , root_tag , value );
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
							add_to_xml( &xml , (int)(ploc - xmlstart) , root_tag , value );
						}
					} /* if (!cform) */
				} /* if ( GET_CH == '<' ) */
			}	
		}
	}while( depth ); 

	xmlsize = (int)(ploc - xmlstart);
	PRINT("Done\n");
	return xml;
}

/* set */
int xml_set(struct XmlData *xml, char *path, char *newval)
{
	struct XmlData *p = xml;

	if (!path || !newval)
		return -1;

	/* XML buffer should exist .. */
	if (!xmlstart)
		return -1;

	while( p ) {
		if (strcmp(p->path,path)) {
			p = p->next;
			continue;
		}
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
		return 0;
	}

	return -1;
}

int xml_write(struct XmlData *xml, char *nmem, int nsize )
{
	struct XmlData *p = xml;
	int nmemp = 0;
	char *buff;
	int xlen = 0;
	char *path;

	if (!xml)
		return -1;

	/* XML buffer should exist .. */
	if (!xmlstart)
		return -1;

	if (!nmem)
		return -1;

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

		XmlFree( p->newval );

		p->newval = NULL;

		return nmemp + (xmlsize - xlen);
	}

	return 0;
}

static char * xml_print(struct XmlData *xml, char *key)
{
	struct XmlData *p = xml;
	int num = 0;
	char *val;

	while( p ) {
		val = p->value ? p->value : "NULL";
		if (!key) {
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
		} else {
			if (!strcmp(p->path,key))
				return val;
		}
		p = p->next;
	}
	return NULL;
}

static void xml_free(struct XmlData *xml)
{
	struct XmlData *q, *p;
	int ix;

	ix = 0;
	p = q = xml;
	while( p ) {
		if( p->next ) 
			q = p->next;
		else 
			q = NULL;
		XmlFree( p );
		++ix;
		p = q;
	}

	PRINT("FREED %d XML PAGE COUNT=%d\n",ix, page_cnt);
	if (page_cnt != -1) {
		/* Free up blocks */
		for(ix = 0;ix < (page_cnt+1);ix++) {
			if (page_root[ix]) {
				PRINT("FREE PAGE %08x\n",(unsigned int)(page_root[ix]));
				freeX(page_root[ix]);
			}
		}
		page_cnt = -1;
	}
		
}

#define NUM_ITEMS 4
static struct pe_update {
	char *epath; /* environment path */
	char *xpath; /* XML path */
} path_env_update[NUM_ITEMS] = 
			{
				{ "ipaddr"		, "lan/ip"		},
				{ "netmask"		, "lan/netmask"	},
				{ "gatewayip" 	, "lan/gateway" },
				{ "serverip"	, "lan/tftpserver" }
			};

/* defined in cmd/nvedit.c */
extern int xenv_set(const char *varname, const char *varvalue);

static void xml_env_update(struct XmlData *xml, struct pe_update *pe)
{
	struct XmlData *p = xml;

	while( p ) {
		if (p->value) {
			if ( !strcmp(p->path,pe->xpath) )
				xenv_set(pe->epath, p->value);
		}
		p = p->next;
	}
}

/*
 *
 *
 * S T O R A G E       B L O C K 
 *
 *
 */
#define _KBYTES(x)	((x)*(1024))
#define _MBYTES(x)	_KBYTES(((x)*(1024)))

#define STORAGE_IO_SIZE		_KBYTES(128)

/* storage header */
#define STORAGE_HEADER_SIZE	STORAGE_IO_SIZE

typedef union {
	struct {
		/* Tragedy (2018/11/08) - 13 young people was dead in a bar in L.A. */
		#define	STORAGE_HEADER_MAGIC		0xdeadb0d1
		unsigned int magic; 	/* magic code 	*/
		unsigned int dirty;	    /* dirty flag   */
		unsigned short crc;   	/* 16bit CRC  	*/
		unsigned short __unused;/* for 32bit align */
		unsigned int start;		/* start offset in storage media */
		unsigned int rsize;		/* actual size of config */
	} __attribute__((packed)) block;
	char dummy[ STORAGE_HEADER_SIZE ];
}__attribute__((packed)) storage_header;

/*
 * NOTICE) 
 * STORAGE_HEADER_SIZE = STORAGE_IO_SIZE = sizeof(storage_header) 
 *
 */

#define STORAGE_HEADER_NUM	4
#define STORAGE_DATA_NUM	2

#define STORAGE_DATA_START	(STORAGE_HEADER_SIZE*STORAGE_HEADER_NUM)
#define STORAGE_DATA_SIZE	_MBYTES(1)
#define STORAGE_SIZE		(STORAGE_DATA_START + (STORAGE_DATA_SIZE * STORAGE_DATA_NUM))

static unsigned long first_data_block( void )
{
	return (unsigned long)(STORAGE_DATA_START);
}

/*
 *
 * P U B L I C       I N T E R F A C E S  
 * P U B L I C       V A R I A B L E S 
 *
 *
 */
static struct {
	struct XmlData *xml;
	storage_header header;
} xcfg = { 0, };

static void enforce_config_block( unsigned long addr )
{
	int ix;
	unsigned long xaddr = addr;
	unsigned long dfblock;
	storage_header *sh;

	dfblock = first_data_block();

	printf("DATA BLOCKS = [0]:%08x  [1]:%08x \n",  
				(int)dfblock , 
				(int)(dfblock + STORAGE_DATA_SIZE) 
			);

	for(ix = 0; ix < STORAGE_HEADER_NUM; ix++) {
		sh = (storage_header *)xaddr;
		memset( (char *)sh, 0, sizeof(storage_header) );

		sh->block.magic = htonl(STORAGE_HEADER_MAGIC);
		sh->block.crc = 0;
		sh->block.__unused = 0;
		sh->block.start = !(ix % 2) ? 
					(unsigned int)dfblock : (unsigned int)(dfblock + STORAGE_DATA_SIZE);
		sh->block.rsize = 0;
		sh->block.dirty = (ix == 0)? 1 : 0;
		sh += 1;
		xaddr += sizeof(storage_header); 
	}
}

/* scanning block */
static unsigned long scan_config_block( unsigned long xaddr , storage_header **xsh ) 
{
	unsigned long addr = xaddr;
	unsigned long daddr;
	int ix;
	storage_header *sh;

	for(ix = 0; ix < STORAGE_HEADER_NUM; ix++) {
		sh = (storage_header *)addr;
		if (ntohl(sh->block.magic) != STORAGE_HEADER_MAGIC) {
			printf("xconfig-header[%d]::ERR::magic %04x\n",ix, ntohl(sh->block.magic));
			{
				#define __LW__ 8
				#define __BSIZE__ 2048
				static char xbuf[ __BSIZE__ ];
				int iy, len;
				unsigned int *p = (unsigned int *)&(sh->block.magic);

				len = 0;
				memset(xbuf,0,__BSIZE__);
				for( iy = 0; iy < (__LW__<<2); iy++ ) {
					len += sprintf(xbuf + len, "%08x ",*p++);
					if ( (iy % __LW__) == (__LW__-1) )
						len += sprintf(xbuf + len, "\n");
				}
				printf("%s\n-----------------------------------------------------\n",xbuf);
			}
			addr += sizeof(storage_header);  /* next header */
			continue;
		}
		if (xsh) 
			*xsh = sh;

		if (sh->block.dirty) 
			break;

		printf("xconfig-header[%d]::OK ::magic %04x\n",ix, ntohl(sh->block.magic));
		addr += sizeof(storage_header); 
	}

	/* DRAM address */
	daddr = xaddr + 
			STORAGE_DATA_START + 
			(ix % STORAGE_DATA_NUM)*STORAGE_DATA_SIZE;

	return (ix == STORAGE_HEADER_NUM) ?  0L : daddr;
}

/*
 * returns
 *
 * xmlval -> none 
 * 			noinit
 *
 *
 */
static int do_xconfig(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr, start;
	struct XmlData *xml = NULL;
	storage_header *sh;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1],"probe")) {
		if (argc < 3)
			return CMD_RET_USAGE;

		/* block location */
		pcaddr = start = addr = simple_strtoul(argv[2], NULL, 16);

		addr = scan_config_block( start , &sh );
		if (addr == (unsigned long)0) {
			env_set("xmlval","none");
			goto __xconfig_probe_failed;
		}

		/* update container */
		if (xcfg.xml) {
			xml_free(xcfg.xml);
			xcfg.xml = NULL;
		}

		xml = xml_parse( xml_header(addr), 0 );
		if (xml) {
			char *s;

			xcfg.xml = xml;
			memcpy( (char *)&xcfg.header, 
							(char *)sh, sizeof(storage_header) ); /* 128Kbytes copy */
			printf("Completed\n");

			/* update environment */
			s = env_get("cfgmaster");
			if (s != NULL) {
				if(!strcmp(s,"xconfig")) {
					int ix;
					/* update .... */
					for(ix = 0;ix < NUM_ITEMS;ix++) 
						xml_env_update( xml, &(path_env_update[ ix ]) );
				}
			}
			
			env_set("xmlval","ok");
			goto __xconfig_ok;
		} 

	__xconfig_probe_failed:
		return CMD_RET_SUCCESS; /* ?? */
	} else
	if (!strcmp(argv[1],"set")) {
		char pamount[16];

		if (argc < 5)
			return CMD_RET_USAGE;

		if (!xcfg.xml) {
			struct XmlData *pxml;
			/* establish basic config block */
			pxml = xml_parse( xml_header( (unsigned long)template_xml ), 0 );
			if (pxml) {
				xcfg.xml = pxml;
				enforce_config_block( pcaddr ); /* initialize config block */
			} else {
			 	env_set("xmlval","noinit");
				goto __xconfig_set_failed;
			}
		}

		memset(pamount,0,16);
		addr = simple_strtoul(argv[2], NULL, 16);
		if( !xml_set(xcfg.xml, argv[3], argv[4])) {
			unsigned long xaddr;
			int amount;
	
			#define _MAX_EDIT_LINE_ 256
			amount = xml_write(xcfg.xml, (char *)addr, xmlsize + _MAX_EDIT_LINE_ );

			/* searching block ... */
			xaddr = scan_config_block( pcaddr , &sh );
			if (xaddr == (unsigned long)0) {
				env_set("xmlval","none");
				goto __xconfig_set_failed;
			}

			memset( (char *)xaddr, 0, STORAGE_DATA_SIZE );
			memcpy( (char *)xaddr, (char *)addr, amount );

			sprintf(pamount,"%d",amount);
			env_set("xmlsize",pamount);

			/* update CRC  && size */
			sh->block.crc = xml_config_crc( (char *)xaddr, amount );
			sh->block.rsize = amount;
		}
	__xconfig_set_failed:
		return CMD_RET_SUCCESS; /* ?? */
	} else
	if (!strcmp(argv[1],"info")) {

		if (!xcfg.xml) {
			env_set("xmlval","noinit");
			goto __xconfig_info_failed;
		}

		sh = (storage_header *)&(xcfg.header);
		if (argc < 3) {
			printf("SIZE=%d, CRC=0x%04x, START=0x%04x\n\n",
					sh->block.rsize, 
					sh->block.crc,
					sh->block.start);
			xml_print(xcfg.xml, NULL);
			env_set("xmlval","all");
		} else {
			char *val = xml_print(xcfg.xml, argv[2] /* key */);
			if (val) {
				env_set("xmlval",val);
				printf("%s\n",val);
			}
		}
		goto __xconfig_ok;

	__xconfig_info_failed:
		return CMD_RET_SUCCESS; /* ?? */
	} else
		return CMD_RET_USAGE;

__xconfig_ok:
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	xconfig, 5, 1, do_xconfig,
	"XML configuration parser>> ",
	"probe [addr]          - probing XML block\n"
	"xconfig info [path]   - print XML information\n"
	"xconfig set  [addr] [path] [value] - Change the value\n"
);
