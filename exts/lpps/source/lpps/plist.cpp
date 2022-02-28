#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fuse.h>

#include "malloc.h"
#include "os.h"
#include "plist.h"
#include "json.h"

using namespace std;

//
//
//
// P R I V A T E     I N T E R F A C E S 
//
//
//

// function to decide whether it is json node or not..
static int node_json( ppsitem * node )
{
	if (!node)
		return 0;

	if (!node->type || !node->value)
		return 0;

	if (!strcmp(node->type,"json"))
		return 1;  // this is json 

	if (!strcmp(node->type,"s") && (node->value[0] == '{'))
		return 1;  // this is also json 

	return 0;
}

// create/expand ppsl item 
static ppsl * lpps_ppsl_create( ppsl ** node, int sz )
{
	int xamount;
      	ppsl *tmp;

	//
	// Precedence rule ?? compiler ?? 
	if (!(*node)) {
		xamount = sizeof(ppsl) + (sizeof(ppsitem) * sz);  // null node ...
		tmp = (ppsl *)os_malloc( xamount );
		if (tmp) {
			tmp->occupancy = 0; // empty list 
			tmp->count = sz;    // size of array 
		}
	} else {
		xamount = sizeof(ppsl) + sizeof(ppsitem) * ((*node)->count + sz); // expand...
		tmp = (ppsl *)os_realloc( (*node) , xamount );
		if (tmp) 
			tmp->count += sz;  // size of array 
	}

	if (!tmp) {
		ERR("%s( %d )::failed\n",(*node)?"os_realloc":"os_malloc",xamount);
		return NULL;
	}

	(*node) = tmp;

	return tmp;
}

// adding an item into ppsl 
static int lpps_ppsl_add( ppsl ** node, char *attr, char *type, char *value )
{
	ppsl *tmp;
	ppsitem *item;

	if (!(*node)) 
		return -1; // NULL LIST 

	tmp = (*node);
	if ((tmp->occupancy+1) >= tmp->count) {
		(*node) = lpps_ppsl_create( node, INCUNIT );
		if ( (*node) == NULL )
			return -1; // NULL LIST 
	}

	tmp = (*node);
	item = (ppsitem *)&(tmp->item[ tmp->occupancy ]);

	item->attrib = attr;
	item->type   = type;
	item->value  = value;

	// debugging ...
	// DBG("  <%s><%s><%s> \n",attr,type,value);

	if (!value)
		DBG(" ++NULL value!!\n");

	tmp->bytelen += print_fmt_len(item->attrib,item->type,item->value); 

	tmp->occupancy ++ ;

	return 0;
}

static __inline__ void lpps_ppsl_sort_swap( ppsitem *item0, ppsitem *item1 )
{
	char *temp;
	//int len;

	/* attribute */
	temp = item0->attrib;
	item0->attrib = item1->attrib;
	item1->attrib = temp;

	/* type */
	temp = item0->type;
	item0->type = item1->type;
	item1->type = temp;

	/* value */
	temp = item0->value ;
	item0->value = item1->value;
	item1->value = temp;
}

void lpps_ppsl_sort( ppsl * node , int start , int end )
{
	int i,j;
	ppsitem *item0, *item1;

	if (!node)
		return ;

	for (i = start; i < end; i++) {
		item0 = (ppsitem *)&(node->item[ i ]);
		for (j = i+1; j <= end; j++) {
			item1 = (ppsitem *)&(node->item[ j ]);
			if (strcmp(item0->attrib,item1->attrib) <= 0)
				continue;
			lpps_ppsl_sort_swap( item0, item1 );
			item0 = (ppsitem *)&(node->item[ i ]);
		}
	}
}

static int lpps_ppsl_find( ppsl * root , ppsitem *b )
{
	int i;
	ppsitem * item;

	if (!b)
		return -1;

	// just looking at attiribute value 
	//
	//if (!b->attrib || !b->type)
	if (!b->attrib)
		return -1;

	for (i = 0; i < root->occupancy; i++) {
		item = (ppsitem *)&(root->item[ i ]);
		if (!item)
			break;

		if (!item->attrib)
			continue;

		if (strcmp(item->attrib,b->attrib)) 
			continue;

		if (strcmp(item->type,b->type))
			continue;

		return i;
	}

	return -1;
}

//
//
//
// P U B L I C       I N T E R F A C E S 
//
//
//

ppsl * lpps_ppsl_parse( ppsl * list, const char * buf, off_t offset, size_t size )
{
	int index = 0;
	const char *sp = buf;
	#define TOKLEN 16
	enum {
		ATTRIB = 0,
		TYPE,
		VALUE,
		TOKTYPE
	};
	struct _tok{
		char * tok;
		int len;
		int rcount; /* realloc count */
	} tokbuf[ TOKTYPE ];
	struct _tok *tok;
	ppsl * node = list;

#define DELIM		':'
#define EOL		'\n'
#define NEXTCHAR	&(buf[ ++index ])

	// 
	// ppsl node create ...
	//
	node = lpps_ppsl_create( &node , INCUNIT );
	if (!node)
		return NULL;

	/* skipping first line */
	if (strchr(sp,'@') != NULL) {
		while (*sp != EOL)
			sp = NEXTCHAR;
		sp = NEXTCHAR;
	} 
	if (!sp) 
		return NULL;

	while (index < (int)size) {
		int i;

		if ( *sp == 0x0 || *sp == ':' ) {
			ERR(" error !! %d/%d \n",index,(int)size);
			goto finish_PPSL_PARSE; // wrong syntax
		}

		for (i = 0; i < TOKTYPE; i++) {
			tokbuf[ i ].tok = (char *)os_malloc( TOKLEN );
			if (!tokbuf[ i ].tok) {
				ERR(" error !! os_malloc( %d )\n",TOKLEN);
				goto finish_PPSL_PARSE; // wrong syntax
			}
			tokbuf[ i ].len = TOKLEN;
		}

		// attribute
		i = 0;
		tok = &(tokbuf[ ATTRIB ]);
		tok->rcount = 0;
		while ( *sp != DELIM && *sp != 0x0 ) {
			tok->tok[ i++ ] = *sp;
			sp = NEXTCHAR;
			if (!sp) 
				goto finish_PPSL_PARSE;
			if (i >= tok->len) {
				tok->len += TOKLEN; 
				tok->tok = (char *)os_realloc( (char *)(tok->tok) , tok->len ); 
				if (!tok->tok) {
					ERR(" os_realloc(ATTRIB,%d)::failed\n",tok->len);
					goto finish_PPSL_PARSE; // wrong syntax
				}
				tokbuf[ ATTRIB ].tok = tok->tok;
				++ tok->rcount;
			}
		}
		tok->tok[ i++ ] = 0;

		// type 
		i = 0;
		sp = NEXTCHAR;
		if (!sp) 
			goto finish_PPSL_PARSE;
		tok = &(tokbuf[ TYPE ]);
		tok->rcount = 0;
		if (*sp == DELIM) {
			/* "::" */
			memcpy( tok->tok , "s", 1 ); /* string... */
			i = 1;
		} else {
			/* type */
			i = 0;
			while ( *sp != DELIM && (*sp != 0x0) ) {
				tok->tok[ i++ ] = *sp;
				sp = NEXTCHAR;
				if (!sp) 
					goto finish_PPSL_PARSE;
				if (i >= tok->len) {
					tok->len += TOKLEN; 
					tok->tok = (char *)os_realloc( (char *)(tok->tok) , tok->len ); 
					if (!tok->tok) {
						ERR(" os_realloc(TYPE,%d)::failed\n",tok->len);
						goto finish_PPSL_PARSE; // wrong syntax
					}
					tokbuf[ TYPE ].tok = tok->tok;
					++ tok->rcount;
				}
			}				
		}
		tok->tok[ i++ ] = 0;

		// value 
		i = 0;
		sp = NEXTCHAR;
		if (!sp) 
			goto finish_PPSL_PARSE;

		tok = &(tokbuf[ VALUE ]);
		tok->rcount = 0;
      		while ((*sp != EOL) && (*sp != 0x0)) {
         		tok->tok[ i++ ] = *sp;
			sp = NEXTCHAR;
			if (!sp) 
				goto finish_PPSL_PARSE;
			if (i >= tok->len) {
				tok->len += TOKLEN; 
				tok->tok = (char *)os_realloc( (char *)(tok->tok) , tok->len ); 
				if (!tok->tok) {
					ERR(" os_realloc(VAL,%d)::failed\n",tok->len);
					goto finish_PPSL_PARSE; // wrong syntax
				}
				tokbuf[ VALUE ].tok = tok->tok;
				++ tok->rcount;
			}
		}
      		tok->tok[ i++ ] = 0;

		sp = NEXTCHAR;
		if (!sp) 
			goto finish_PPSL_PARSE; // wrong syntax

		for (i = 0; i < TOKTYPE; i++)
			tokbuf[ i ].len = 0;

		// adding a record ...
		if ( lpps_ppsl_add( &node, 
				tokbuf[ ATTRIB ].tok , tokbuf[ TYPE ].tok , tokbuf[ VALUE ].tok ) ) {
			ERR(" lpps_ppsl_add( %s, %s, %s )::failed\n",
				tokbuf[ ATTRIB ].tok , tokbuf[ TYPE ].tok , tokbuf[ VALUE ].tok );
			goto finish_PPSL_PARSE; // wrong syntax
		}
	} /* while (index < (int)size) */

	return node;

finish_PPSL_PARSE:
	lpps_ppsl_remove( &node );
	return NULL;
}

// read procedure 
int lpps_ppsl_dump( ppsl * node, char * buff, int size, int json , ppriv * priv )
{
	int pamount;
	int curamt = 0;  // data ...
	ppsitem *tmp;
	int plen;
	int count = 0;

	// #1. sanity check
	if (!node || !priv)
		return 0; // NULL LIST 

	// #2. saity check  
	if (!node->occupancy)
		return 0; // NO data contents as well 

	for (; priv->rptr < node->occupancy; priv->rptr++) {
		tmp = (ppsitem *)&(node->item[ priv->rptr ]);

		++ count;

		// option processing 
		if (json && node_json(tmp)) {
			// json node 

			sjson_table_t jtbl;

			// json parsing..
			sjson_parse( &jtbl , (unsigned char *)tmp->value );

			// beginning new line 
			sprintf(priv->linebuf, "\n" );

			// dumping to temporary buffer
			plen = sjson_print_json( &jtbl, priv->linebuf+1 /*"\n"*/, priv->buflen-2 );

			sjson_free( &jtbl );
		} else {
			// just dumping to temporary buffer
			plen = snprintf(priv->linebuf,priv->buflen-2,"%s",tmp->value);
		}

		priv->linebuf[ plen ] = 0x0; // null termination 
		pamount = snprintf(buff+curamt,(size_t)(size-curamt),
					print_fmt,tmp->attrib,tmp->type,priv->linebuf);

		if ((size_t)(size-curamt) < pamount) // ? CLUMSY CODE ..
			break;

		curamt += pamount; 
	}

	//
	// Ending '\n' processing - very strange - needs to be revised
	//
	if (buff[ curamt-1 ] != '\n')
		buff[ curamt-1 ] = '\n'; // return code 
	buff[ curamt ] = 0x0 ;           // null termination 

	DBG(" READ DUMP %d/%d %d \n",priv->rptr,node->occupancy,curamt);

	return curamt;
}

// copying a file 
ppsl * lpps_ppsl_duplicate( ppsl * node )
{
	int i;
	ppsl *tmp;
	ppsl *dummy = NULL;
	ppsitem *item;

	if (!node) 
		return NULL; // NULL LIST 

	tmp = lpps_ppsl_create( &dummy , INCUNIT );
	if (!tmp)
		return NULL;

	for (i = 0; i < node->occupancy; i++) {
		item = (ppsitem *)&(node->item[ i ]);
		lpps_ppsl_add( &tmp , 
				os_strdup( item->attrib ) , 
				os_strdup( item->type ), 
				os_strdup( item->value ) );
	}

	return tmp;
}

// copying a file 
int lpps_ppsl_concat( ppsl ** node , ppsl * appended )
{
	int i;
	ppsitem *item;

	if (!node || !(*node))
		return -1; 

	if (!appended) 
		return 0; // NULL LIST 

	for (i = 0; i < appended->occupancy ; i++) {
		item = (ppsitem *)&(appended->item[ i ]);
		lpps_ppsl_add( node , 
			os_strdup( item->attrib ) , 
			os_strdup( item->type ), 
			os_strdup( item->value ) );	
	}

	return 0;
}

// debugging output 
int lpps_ppsl_print( ppsl * node )
{
	int i;
	ppsitem *tmp;

	if (!node) 
		return -1; // NULL LIST 

	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	for (i  = 0; i < node->occupancy; i++) {
		tmp = (ppsitem *)&(node->item[ i ]);
		//printf(print_fmt, tmp->attrib, tmp->type, tmp->value);
		printf("<%s>:<%s>:<%s> \n",tmp->attrib, tmp->type, tmp->value);
	}
	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

	return 0;
}

// removing ppsl block 
int lpps_ppsl_remove( ppsl ** node )
{
	int i;
	ppsitem *item;
	
	if (!node) 
		return -1; // NULL LIST 

	for (i = 0; i < (*node)->occupancy; i++) {
		item = (ppsitem *)&((*node)->item[ i ]);
		if (item->attrib) {
			os_free( item->attrib );
			item->attrib = NULL;
		}
		if (item->type) {
			os_free( item->type );
			item->type = NULL;
		}
		if (item->value) {
			os_free( item->value );
			item->value = NULL;
		}
	}
	os_free( (*node) );
	(*node) = NULL;

	return 0;
}

// merging two lists 
ppsl * lpps_ppsl_merge( ppsl * root , ppsl * node , ppsl ** delta )
{
	int i, j;
	ppsitem *item, *ritem;
	int olen, nlen;
	
	if (!root || !node)
		return NULL; // NULL

	/* delta ready */
	if (delta) {
		//
		// Delta packet is post-cleaned simply to support multiple reader access.
		//
		if ( (*delta) )
			lpps_ppsl_remove( delta );

		(*delta) = lpps_ppsl_create( delta, INCUNIT );
	}

	for (i = 0; i < node->occupancy; i++) {
		item = (ppsitem *)&(node->item[ i ]);
		if (!item->attrib)
			continue;

		j = lpps_ppsl_find( root , item );
		if (j < 0) {
			DBG(" add [ %s:%s:%s ] \n",item->attrib,item->type,item->value);
			lpps_ppsl_add( &root , 
					os_strdup( item->attrib ), 
					os_strdup( item->type   ), 
					os_strdup( item->value  ) );

			if (delta)
				lpps_ppsl_add( delta,
					os_strdup( item->attrib ),
					os_strdup( item->type   ),
					os_strdup( item->value  ) );
			continue;
		}

		// same data !! 
		
		ritem = (ppsitem *)&(root->item[ j ]);

		olen = strlen(ritem->value);  // old length 
		DBG(" replace-[ %s:%s:%s ] - %d \n", item->attrib, item->type, item->value,olen);

		// replace value 
		os_free( ritem->value );
		ritem->value = os_strdup(item->value);

		nlen = strlen(ritem->value);  // new length 
		DBG(" replace+[ %s:%s:%s ] - %d \n",ritem->attrib,ritem->type,ritem->value,nlen);

		if (delta)
			lpps_ppsl_add( delta, 
				os_strdup( ritem->attrib ),
				os_strdup( ritem->type   ),
				os_strdup( ritem->value  ) );

		// update root size 
		root->bytelen -= olen;
		root->bytelen += nlen;	
	}

	//
	//if (delta)
	//	lpps_ppsl_print( *delta );

	return root;
}

