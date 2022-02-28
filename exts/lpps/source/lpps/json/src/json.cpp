#include "json.h"

using namespace std;

/*
 * Parse tree maintaines an intermediate node to keep the semantics of block.
 * Just for node BLK/SET.
 *
 */
#define interm_node(node)	((node)->value)
#define term_token(node)	(!((node)->value) && !((node)->bros))
#define multiple_node(node)	(((node)->type == BLOCK) || ((node)->type == SET))
#define empty_node(node)	(!((node)->value) && !((node)->bros))
#define symbol_name(x)		((!(x))?"NULL":(x)->symbol)

#define STRROOM		4

/* 
 *
 * ---------- O P T I O N S                      ------------------- 
 *
 */
//#define VERBOSE_ADD
#define CONDENSED_DISPLAY_TREE

/* 
 *
 * ---------- G L O B A L      V A R I A B L E S ------------------- 
 *
 */
u32 json_dbg = 0;


/* 
 *
 * ---------- P R I V A T E    T Y P E S          ------------------- 
 *
 */

/* 
 *
 * ---------- P R I V A T E    D E C L A R A T I O N s -------------- 
 *
 */
static void __sjson_free( sjson_node_t *node );
static void __sjson_free_node( sjson_node_t *node );

/* 
 *
 * ---------- P R I V A T E    F U N C T I O N S ------------------- 
 *
 */
static 
s8 * __node_name( sjson_node_t *node )
{
	if (!node)
		return (s8 *)"NULL";

	switch (node->type) {
	case BLOCK:
		return (s8 *)"BLK ";
	case SET:
		return (s8 *)"SET ";
	case TOKEN:
		return (s8 *)"TOKN";
	case STMT:
		return (s8 *)"STMT";
	default:
		return (s8 *)"UNKN";	
	}
}

static __inline__
sjson_node_t *__sjson_new_node( int type, int lvl, s8 *name )
{
	sjson_node_t *temp;

	temp = (sjson_node_t *)os_malloc( sizeof( sjson_node_t ) );
	if (!temp)
		return NULL;

	temp->type 	= type;
	temp->lvl 	= lvl;
	temp->symbol 	= name;
	temp->value 	= NULL; /* children = lvl+1 */
	temp->bros  	= NULL;	/* brothers = lvl   */
	temp->prev 	= NULL;	/* parent   = lvl-1 */
	temp->flag	= 0;    /* zeroing */

	return temp;
}

static
int  __sjson_max_depth( sjson_node_t * node , int level )
{
	static int mlvl = 0;

	if (!node)
		return 0;

	__sjson_max_depth( node->value , level+1 );
	__sjson_max_depth( node->bros  , level );

	mlvl = (mlvl < level) ? level : mlvl;
       
	return mlvl;
}

static __inline__  
int __sjson_set_member( sjson_node_t * node, sjson_node_t *tnode, int *nth )
{
	int dset = 0;
	sjson_node_t * pnode = NULL;

	/* direct set is always exceptional case */
	/* Each token number in a set is only token node */
	/*
	 *  SETx 
	 *    SETy
	 *      TOKEN ...  <- node
	 */
	if (node->prev) {
		pnode = node->prev;
		if (pnode->type == SET) 
			dset = 1;
	}

	if (!dset)
		return 0;

	/* dset == 1 && !pnode */
	if (nth && tnode) {
		sjson_node_t * p = node->bros;
		int cnt = 0;
		while (p) {
			if (p == tnode)
				break;
			++cnt;
			p = p->bros;
		}

		*nth = cnt; /* n'th item */
	} 

	return 1;
}

static 
void __sjson_print_lvl( sjson_table_t *tbl, int index )
{
	sjson_node_t * p;
	int i;
	s8 temp[8];

	if (!json_dbg)
		return ;

	for( i = 0; i < index; i++ ) {
		p = tbl->root[ i ];
		if (!p)
			continue;
		printf("\t* %d * < ",i);
		while (p) {
			memset(temp,0,8);
			if (p->type == TOKEN)
				snprintf(temp,7,"TOKEN");
			else 
				snprintf(temp,7,"%s",symbol_name(p));
			printf("%-8s ",temp);
			p = p->bros;
		}
		printf(" >\n");
	}
	printf("\n");
}

/*
 * STMT processing ...
 */
static
sjson_node_t *__sjson_proc_stmt( sjson_table_t *tbl, sjson_node_t * temp, int lvl ) 
{
	sjson_node_t *p, *t;
	int upd_root =0;

	/* check !! */
	if (tbl->root[ lvl ]) {
		sjson_node_t *v = tbl->root[ lvl ];
		/* 
	 	* Very important sidekick !!
	 	*/
		if (v->type == SET) {
			sjson_node_t *ip = tbl->last[ lvl ];
			__sjson_free_node(temp); temp = NULL;

			ip->prev = v; /* all the same parent */

			JDBG("\tREMOVE SET NODE \n");
			goto __skip_set_block;
		}
	}

	p = (sjson_node_t *)tbl->last[ lvl ];
	t = p->prev; 		/* find the second last node */
	t->bros = NULL;		/* unlink p & t */
	p->bros = NULL;		/* make them be orphans */

	upd_root = 0;
	if (t == tbl->root[ lvl ]) 
		upd_root = 1;   /* table node update required */

	temp->value = t;
	temp->prev  = t->prev;
	temp->bros  = NULL; 

	if (t->value) {
		ERR("\nBusy parse node value\n\n");
		return NULL;
	}

	t->value    = p;
	t->prev     = 
	p->prev     = temp;

	if (temp->prev)
		temp->prev->bros = temp; /* Now I am brother */

	if (upd_root)
		tbl->root[ lvl ] = temp;

	tbl->last[ lvl ] = temp;  /* update last pointer */

	JDBG("[%s     %d  %s] -> %s %s <%s> <%s> \n",
		__node_name(temp),lvl,
			symbol_name(temp),symbol_name(t),symbol_name(p),
			__node_name(t),__node_name(t->value));
  
  __skip_set_block:
	__sjson_print_lvl( tbl, MAXL );

	return temp;
}

/*
 * __sjson_add_node - 
 * 	Creates a node for a token 
 *
 */
static 
sjson_node_t *__sjson_add_node( sjson_table_t *tbl, int type, int lvl, s8 *name ) 
{
	sjson_node_t *temp, *p, *t;

	if (lvl < 0) {
		if (name)
			os_free( name );
		return NULL;
	}

	/* new node */
	temp = __sjson_new_node( type, lvl, name );
	if (!temp) {
		if (name)
			os_free( name );
		return NULL;
	}

	if (lvl >= MAXL) {
		ERR("Too much big depth !!\n"); /* CRITICAL ERROR */
		if (name)
			os_free( name );
		return NULL;
	}

	switch (type) {
	case SET:
	case BLOCK:
		JDBG("[%s     %d  %s ]\n",__node_name(temp),lvl,name?name:"none");
		if (tbl->lvl < lvl) {
			/* terminating previous stage */
			p = (sjson_node_t *)tbl->last[ lvl-1 ];

			if (tbl->root[ lvl ]) {
				p = (sjson_node_t *)tbl->last[ lvl ];
				p->bros = temp;
				temp->prev = p;
				tbl->last[ lvl ] = temp; 
			} else {
				tbl->root[ lvl ] = 
				tbl->last[ lvl ] = temp;
			}
		} else {
			/* current level */
			p = tbl->last[ lvl ];
			if (!p) {
				ERR("parse::block error [%d/%d]\n",lvl,tbl->lvl);
				return NULL;
			}
	
			p->bros = temp;
			temp->prev = p;
			tbl->last[ lvl ] = temp;
			temp->value = tbl->root[ lvl+1 ];

			/* prev pointer */
			p = tbl->root[ lvl+1 ];
			p->prev = temp;

			/* clear high level */
			tbl->root[ lvl+1 ] = 
			tbl->last[ lvl+1 ] = NULL;
		}
		__sjson_print_lvl( tbl, MAXL );
		break;
	case TOKEN:
		JDBG("[%s     %d  %s ]\n",__node_name(temp),lvl,name?name:"none");
		if (!tbl->root[ lvl ]) {
			tbl->root[ lvl ] = 
			tbl->last[ lvl ] = temp;
		} else {
			p = (sjson_node_t *)tbl->last[ lvl ];
			p->bros = temp;
			temp->prev = p;
			tbl->last[ lvl ] = temp; /* update last pointer */
		}
		__sjson_print_lvl( tbl, MAXL );
		break;
	case STMT:
		p = (sjson_node_t *)tbl->last[ lvl ];
		if (!p) {
			ERR("parse::state error 1\n");
			return NULL;
		}
		t = p->prev; 		/* find the second last node */
		if (!t) {
			/* '{}' --> possible case !! */
			return NULL;
		}
		__sjson_proc_stmt( tbl, temp, lvl );
		break;
	case COMMA:
		JDBG("[%s     %d  %s ]\n",__node_name(temp),lvl,name?name:"none");
		p = (sjson_node_t *)tbl->last[ lvl ];
		if (p) {
			sjson_node_t *q = p->value;

			/* Comma Tagging... */
			if (q) {
				while (q->value)
					q = q->value;
				q->flag |= F_COMMA;
			} 
		}
		__sjson_free_node( temp );
		break;
	default:
		ERR("???? %d %s %p \n",lvl,name,temp);
		break;
	}

	tbl->lvl = lvl; /* keeping the variation of level */

	return temp;
}

static 
int __sjson_tree_print_internal( sjson_node_t *node , int lvl , int maxlvl , s8 *buff , s32 size )
{
	int i;
	int tlen = 0;
	int len = 0;
	int err = 0;

	if (!node)
		return 0;

	if (!buff || (size <= 0))
		return 0;

	#define TJDBGP(fmt,args...) \
		{ \
		tlen = snprintf((char *)buff+len,                     \
				(len>size)?0:(size-len),fmt,##args);  \
		len = (tlen == 0)? size : (len + tlen);               \
		err = (tlen == 0)? 1 : 0;                             \
		}

	TJDBGP("\tP %-3d   %-3d  ",lvl,node->lvl);
	for (i = 0; i < (node->lvl); i++) 
		TJDBGP("  ");
	TJDBGP("<%-5s> %-20s",__node_name(node), symbol_name(node));
	for (i = 0; i < (maxlvl-node->lvl); i++)
		TJDBGP("  ");
	TJDBGP("%d %c <%-20s> <%-20s> <%-20s>\n",
			node->flag & F_COMMA ? 1: 0,
			node->flag & F_DEL   ? '*':' ',
			node->value ? symbol_name(node->value):" ",
			node->bros  ? symbol_name(node->bros) :" ",
	     		node->prev  ? symbol_name(node->prev) :" " );

	len += __sjson_tree_print_internal(node->value , lvl+1 , maxlvl , buff+len, size-len ); /* value - lower level */
	len += __sjson_tree_print_internal(node->bros, lvl , maxlvl , buff+len, size-len );  /* brothers - same level */

#ifndef CONDENSED_DISPLAY_TREE
	if ( multiple_node(node) ) {
		TJDBGP("\tE %-3d   %-3d  ",lvl,node->lvl);
		for (i = 0; i < (node->lvl); i++) 
			TJDBGP("  ");
		TJDBGP("<%-5s> %-20s",__node_name(node), symbol_name(node));
		for (i = 0; i < (maxlvl-node->lvl); i++)
			TJDBGP("  ");
		TJDBGP("%d %c <%-20s> <%-20s> <%-20s>\n",
				node->flag & F_COMMA ? 1: 0,
				node->flag & F_DEL   ? '*':' ',
				node->value ? symbol_name(node->value):" ",
				node->bros  ? symbol_name(node->bros) :" ",
	     			node->prev  ? symbol_name(node->prev) :" " );
	}
#endif 

	return err ? -1 : len;
}

static sjson_node_t * __sjson_json_lookback( sjson_node_t * node , int *step , int cnt )
{
	int i;
	sjson_node_t *p;

	if (!node)
		return NULL;

	p = node->prev;
	for (i = 0; i < cnt; i++) {
		if (!p)
			return NULL;

		if (p->type != (u32)step[i]) 
			return NULL ;
		p = p->prev;
	}

	return p;
}

/*------------------------------------
 *
 *  J S O N     Printout Function 
 *
 */
static 
int __sjson_json_print_internal( sjson_node_t *node , s8 *buff, int size, int space )
{
	sjson_node_t *p;
	sjson_node_t *lb; /* look back */
	int blks[ 2 ] = { BLOCK, STMT };
	int sets[ 2 ] = { SET, STMT };
	int i, tlen, len = 0;

	if (!node)
		return 0;

	#define JDBGP(fmt,args...) \
		{ \
		tlen = snprintf((char *)buff+len,                     \
				(len>size)?0:(size-len),fmt,##args);  \
		len = (tlen == 0)? size : (len + tlen);               \
		}
	#define DOSPACE() \
		do{ if (space) { for (i = 0; i < node->lvl; i++) JDBGP("  "); } }while(0)
	#define DOSPACE2() \
		do{              for (i = 0; i < node->lvl+1; i++) JDBGP("  "); }while(0)
		

	/* check intermediate node */
	if (multiple_node(node)) {
		if (interm_node(node)) 
			goto __skip;
	}
	
	switch( node->type ) {
	case BLOCK:
		lb = __sjson_json_lookback( node, blks, 2 );
		if (!lb)
			DOSPACE();
		else 
			JDBGP(" ");
		JDBGP("{\n");
		break;
	case SET:
		lb = __sjson_json_lookback( node, sets, 2 );
		if (!lb)
			DOSPACE();
		else 
			JDBGP(" ");
		JDBGP("[");
		if (node->bros) {
			p = node->bros;
			if (p->type != TOKEN)
				JDBGP("\n");
		}
		break;
	case STMT:
		break;
	case TOKEN:
		if (node->value)
			DOSPACE();
		if (__sjson_set_member( node, NULL, NULL )) {
			sjson_node_t *s = node->prev;
			/*
			 * Special case - "[ "a", { "b":"c" }, "d" ... ]"
			 * Just to make the indented printout of "d" 
			 */
			while (s && (s->bros != node))
				s = s->bros;
			if ((s->type != TOKEN) 
					&& (s->type != SET)) {
				DOSPACE2();  /* Never overlapped with 
					     the upper case " if (node->value) .... " */
			}
		}
		if (node->symbol) {
			if (node->flag & F_DEL)
				JDBGP("@"); /* deleted node */
			if (node->value) {
				JDBGP("\"%s\": ",node->symbol);
			} else {
				JDBGP("\"%s\"",node->symbol);
			}
		}
		if (__sjson_set_member( node, NULL, NULL )) {
			if (node->bros) {
				sjson_node_t * bros = node->bros;
				JDBGP(", ");  /* only !node->value */
				if (bros->type != TOKEN) 
					JDBGP("\n");
			} 
		}
		break;
	default:
		JDBGP("UNKNOWN");
		break;
	}

  __skip:
	len += __sjson_json_print_internal( node->value, buff+len, size-len, 1 );
	len += __sjson_json_print_internal( node->bros,  buff+len, size-len, 0 );

	/* check intermediate node */
	if (multiple_node(node)) {
		if (interm_node(node)) 
			return len;
	}
	
	switch (node->type) {
	case BLOCK:
		DOSPACE();
		if (node->flag & F_COMMA) {
			JDBGP("},");
		} else {
			JDBGP("}");
		}
		JDBGP("\n");
		break;
	case SET:
		if (node->bros) {
			sjson_node_t *q = node->bros;
			/* end of SET element */
			/* [ "A" , { "a": "b"} , "B" ] */
			while (q->bros) 
				q = q->bros; 
			if (q->type != TOKEN) {
				DOSPACE();
			}
		}
		if (node->flag & F_COMMA) {
			JDBGP("],\n");
		} else {
			JDBGP("]\n");
		}
		break;
	case TOKEN:
		if (!__sjson_set_member( node, NULL, NULL )) {
			/* SET: [ "a", "b", "c",,, ] */
			if (term_token(node)) {
				if (node->flag & F_COMMA) {
					JDBGP(",\n");
				} else {
					JDBGP("\n");
				}
			}
		} 
		break;
	}

	return len;
}

/*------------------------------------
 *
 *  X M L     Printout Function 
 *
 */
static int json_xml_setelm = 0;
static int json_xml_setlvl = 0;
/* SET attribute is converted into set of XML field with a name beginning with '_elem' */

static __inline__ sjson_node_t * ___sjson_xml_set_included( sjson_node_t *node )
{
	sjson_node_t *p;
	sjson_node_t *lb;

	lb = p = node;
	while (p) {
		if (p->type == SET)
			return lb;
		lb = p;
		p = p->prev;
	}

	return NULL; /* last block node */
}

static 
int __sjson_xml_print_internal( sjson_node_t *node , s8 *buff, int size, int space )
{
	int i, tlen, len = 0;
	sjson_node_t *v, *parent;

	if (!node)
		return 0;

	#define _JDBGP(fmt,args...) \
		{ \
		tlen = snprintf((char *)buff+len,                     \
				(len>size)?0:(size-len),fmt,##args);  \
		len = (tlen == 0)? size : (len + tlen);               \
		}
	#define _DOSPACE(space) \
		do{ if (space) { for (i = 0; i < (node->lvl+1+json_xml_setlvl); i++) _JDBGP("  "); } }while(0)
	#define _DOSPACE2(space) \
		do{ if (space) { for (i = 0; i < (node->lvl+json_xml_setlvl); i++) _JDBGP("  "); } }while(0)
		

	/* check intermediate node */
	if (multiple_node(node)) {
		if (interm_node(node)) 
			goto __skip;
	}
	
	switch( node->type ) {
	case BLOCK:
		if ( ___sjson_xml_set_included( node ) ) {
			_DOSPACE2(1); /* Just do it */
			_JDBGP("<%s%d>\n",
				JSON_XML_SET_NAME, json_xml_setelm);
		}
		break;
	case SET:
		json_xml_setelm = 0;
		json_xml_setlvl ++ ;
		break;
	case STMT:
		break;
	case TOKEN:
		{
		int set_parent = 0; /* TODO - CLUMSY CONTROL */

		if (!node->symbol)
			break;		/* non-symbol */
		v = node->value;
		if (v) 
			_DOSPACE(space);	/* first symbol in STMT */

		/* Check if it is on the set */
		set_parent = 0;
		parent = node->prev;
		if (parent)
			set_parent = (parent->type == SET) ? 1 : 0;

		if (set_parent) {
			_DOSPACE(1); /* Just do it */
			_JDBGP("<%s%d>%s</%s%d>\n",
					JSON_XML_SET_NAME,
					json_xml_setelm,
					symbol_name(node),
					JSON_XML_SET_NAME,
					json_xml_setelm );
			++json_xml_setelm;
		} else {	
			if (v) {
				_JDBGP("<%s>",symbol_name(node));
				if (v->type != TOKEN)
					_JDBGP("\n");
			} else
				_JDBGP("%s",symbol_name(node));
		}
		}	
		break;
	default:
		_JDBGP("UNKNOWN");
		break;
	}

  __skip:
	len += __sjson_xml_print_internal( node->value, buff+len, size-len, 1 );
	len += __sjson_xml_print_internal( node->bros,  buff+len, size-len, 0 );

	/* check intermediate node */
	if (multiple_node(node)) {
		if (interm_node(node)) 
			return len;
	}
	
	switch (node->type) {
	case TOKEN:
		if (node->value) {
			if (multiple_node(node->value))
				_DOSPACE(space);
			_JDBGP("</%s>\n",symbol_name(node));
		}
		break;
	case SET:
		json_xml_setlvl -- ;
		break;
	case BLOCK:
		if ( ___sjson_xml_set_included( node ) ) {
			_DOSPACE2(1); /* just do it ??? */
			_JDBGP("</%s%d>\n",
				JSON_XML_SET_NAME, json_xml_setelm);
			++ json_xml_setelm;
		}
		break;
	case STMT:
		break;
	}

	return len;
}

/* Deleting a single node */
static 
void __sjson_free_node( sjson_node_t *node )
{
	if (!node)
		return ;

	if (node->symbol)
		os_free( node->symbol );
	os_free( node );
}

/* Removing a tree */
static
void __sjson_free( sjson_node_t *node )
{
	if (!node)
		return;

	/* post-order traverse */
	if (node->bros) {
		__sjson_free( node->bros );
		node->bros = NULL;
	}
	if (node->value) {
		__sjson_free( node->value );
		node->value = NULL;
	} 

	__sjson_free_node(node);
}

static 
s8 * __sjson_snm( int code , int *num )
{
	char buff[64];

	memset(buff, 0, 64);

	switch (code) {
	case BLOCK: sprintf(buff,"BLK%d",(*num)); ++ (*num); return os_strdup(buff);
	case SET  : sprintf(buff,"SET%d",(*num)); ++ (*num); return os_strdup(buff);
	case STMT : sprintf(buff,"SMT%d",(*num)); ++ (*num); return os_strdup(buff);
	default	  : sprintf(buff,"XXX%d",(*num)); ++ (*num); return os_strdup(buff);
	}
}

/* searching specific node in a subtree */
static 
sjson_node_t * __sjson_scan_node( sjson_node_t * node , char *sn )
{
	sjson_node_t * found;

	if (!node)
		return NULL;

	found = NULL;
	if (node->type == TOKEN) {
		if (!strcmp(symbol_name(node),sn))
			found = node;
	}

	/* TODO : too much language dependent code */
	return (sjson_node_t *)(
			(long)__sjson_scan_node( node->value , sn ) | 
			(long)__sjson_scan_node( node->bros  , sn ) | 
			(long)found
		);
}

/* ???????????? - complicated  
 */
static __inline__
s32 __sjson_calc_set_order( sjson_node_t *stmt /* set starting */ , sjson_node_t * tok )
{
	sjson_node_t *t;
	sjson_node_t *v;
	s32 cnt;

	if (!tok)
		return -1;

	if (!tok->value)
		return -1;

	/* looking for set */
	t = stmt->value;
	while (t) {
		if (t->type == SET && !t->value)
			break;
		t = t->value;
	}

	if (t) {
		if (t->type != SET) {
			ERR("\t\tt=%s error\n",symbol_name(t));
			return -1;
		}
	} else {
		ERR("\t\tt=NULL error\n");
		return -1;
	}

	cnt = 0;
	t = t->bros; /* start scan inside of set */
	/* if t is single token --> 
	 * 	Value is null 
	 */
	while (t) {
		v = t->value;
		/* JDBG("\t\t\t\tS:v=%s \n",symbol_name(v)); */
		if (!v) {
			/*
	 		* SINGLE TOKEN DISCRIMINATION !!
	 		*
	 		* "aa": [
	 		* 	"maggie",  <-- SINGLE TOKEN  !!  
	 		* 	{ "jimmie": "flashback" },
	 		* 	{ "tonny":  "member1" }
	 		*  ]
	 		*
	 		*/
			/* single token !! */
			/* Just counting and skipped !! */
			t = t->bros;
			cnt++;  /* next element */
			continue;
		}
		if (v->type == BLOCK) {
			/*
			 * Inside block - many of items might exist... 
			 * We need to iterate these items...
			 */
			sjson_node_t * x, *n = NULL;

			while (v->value)
				v = v->value;
			v = v->bros;
			x = v;
			while ( x ) {
				if (x->type == STMT)
					n = x->value;
				if (n) {
					if (n == tok->value)
						return cnt;
				}
				x = x->bros;
			}
		}
		t = t->bros;
		cnt++;  /* next element */
	}
	return -1;
}

static 
s8 * __sjson_path_trim_index( s8 * path )
{
	s8 * trim;
	s8 * temp = (s8 *)os_malloc( strlen(path) + 1 );

	/* indexing backet is not included in the symbol name. 
	 * We fail in searching a node with indexed name string */
	if (!temp)
		return NULL;

	memset(temp, 0, strlen(path)+1);
	strcpy(temp,path);

	trim = strrchr(path,'['); /* first trial - [ */
	if (trim)
		temp[ trim - path ] = 0x0; /* trimming */

	/* inside check again */
	/* a[0].l1.l2.l3.l4. xxxxx - It should be required */
	/* string variable --> should be "temp" not "path" */
	trim = strchr(temp,'.'/*JSON_PATH_DELIM*/); /* second trial */
	if (trim)
		temp[ trim - temp ] = 0x0; /* trimming */

	return temp;
}

static
s8 * __sjson_path_name_internal( sjson_node_t * tnode , int dset )
{
	sjson_node_t *p;
	s8 *path = NULL;
	s32 incset = 0; /* included in a set ?? */ 
	s32 slvl = 0; 	/* the order in a set */

	/* tnode -> should be "TOKEN" type node - beginning .. */

	if (!tnode)
		return NULL;

	p = tnode;

	//JDBG("%s: \t^^^^^^^^^^^^^^^^^^^^^^^^^^^^ \n",__func__);
	while (p) {
		if (!dset) {
			/* 
			 * Token may be the first level element of a set (dset == 1),
			 * In this case, this routine should be skipeed. 
			 */
			while (!multiple_node(p)) {  /* STMT */
				p = p->prev;
				if (!p)
					goto __end_of_path;
			}

			while (multiple_node(p)) {   /* BLK/SET */
				if (p->type == SET)
					incset = 1;  /* Checks whether... */
				p = p->prev;
				if (!p) 
					goto __end_of_path;
			}
		} 

		if (p->type == STMT) {
			int sz = strlen(symbol_name(p->value))+STRROOM;
			if (!path) {
				path = (char *)os_malloc(sz);
				if (!path)
					return NULL;
				memset(path,0,sz);
				strcpy(path,symbol_name(p->value));
				if (incset) {
					slvl = __sjson_calc_set_order( p, tnode );
					if (slvl >= 0) 
						sprintf(path+strlen(path),"[%d]",slvl);
				}
			} else {
				int o_sz = strlen(path)+STRROOM;
				char *temp = (char *)os_malloc(o_sz+sz);
				if (!temp) 
					return NULL;
				memset(temp,0,o_sz+sz);
				strcpy(temp,symbol_name(p->value));
				if (incset) {
					s8 *xpath;
					sjson_node_t *m;
					xpath = __sjson_path_trim_index( path ); /* hello[0] --> hello */
					if (!xpath) {
						ERR("FATAL error - path triming error\n");
						os_free(temp);
						return NULL; /* FATAL error */
					}
					m = __sjson_scan_node( p , xpath );
					if ( m ) {
						slvl = __sjson_calc_set_order( p, m->prev );
						if (slvl >= 0)
							sprintf(temp+strlen(temp),"[%d]",slvl);
					} 
					if (xpath)
						os_free(xpath);
				}
				strcat(temp,JSON_PATH_DELIM);
				strcat(temp,path);
				os_free(path);
				path = temp;
			}
		}
		//JDBG("%s: \t\t path step = <<%s>>\n",__func__,path);
		p = p->prev;

		dset = 0;
	__end_of_path:
		do{}while(0);
		incset = 0;
	}
	//JDBG("%s: [ %s ]\n",__func__, path);
	return path;
}

static
s8 * __sjson_path_name( sjson_node_t * tnode /* should be token node */ )
{
	sjson_node_t *p;
	s8 *path;
	int dset;
	int cnt = -1;

	if (!tnode)
		return NULL;

	/* first stage */
	p = tnode->prev;
	if (!p) {
		ERR("!prev at token\n");
		return (char *)symbol_name(tnode);
	}

	/* check set element items "[ "a", "b", "c", ... ]" */
	/* SET index */
	dset = __sjson_set_member( p, tnode, &cnt );

	while ( multiple_node(p) )
		p = p->prev;

	if (p->type != STMT) {
		ERR("!stmt/block at token (%s) (%s)\n",__node_name(p), symbol_name(p));
		return (char *)symbol_name(tnode);
	}

	path = __sjson_path_name_internal( p , dset );
	if (path) {
		int sz = strlen(symbol_name(tnode))+STRROOM;
		int o_sz = strlen(path)+STRROOM;
		char *temp = (char *)os_malloc(o_sz+sz);
		if (!temp) 
			return NULL;
		
		memset(temp,0,o_sz+sz);
		strcpy(temp,path);
		if (cnt != -1)
			sprintf(temp+strlen(temp),"[%d]",cnt);
		else {
			strcat(temp,JSON_PATH_DELIM);
			strcat(temp,symbol_name(tnode));
		}
		os_free(path);
		return temp;
	} else {
		return os_strdup(symbol_name(tnode));
	}
}

static
s8 *__nonindex_string( s8 *s , int set_element )  
{
	s8 *p, *q;
	s8 *ns;
	int sz;
	int insub;

	if (!s)
		return NULL;

	sz = strlen(s);
	ns = (s8 *)os_malloc(sz+1);
	if (!ns)
		return NULL;

	memset(ns,0,sz+1); /* clear */

	/* SET index notation is eliminated ... */

	/* LAST SET INDEX */
	/* 
	 *
	 *  AA.BB[0].CC.DD.ZZ       --> AA.BB.CC.DD.ZZ
	 *  AA.BB[0].CC.EE          --> AA.BB.CC.EE
	 *  AA.BB[0].CC.EE[0]       --> AA.BB.CC.EE[0]  (set_element = 1)
	 *  AA.BB[0].CC.EE[0]       --> AA.BB.CC.EE     (set_element = 0)
	 *
	 */

	p = s;	
	q = ns;
	insub = 0;
	while ( *p ) {
		if (*p == '[') {
			if (set_element) {
				if (!strchr(p,'.')) {  
					*q++ = *p;   /* FINAL ELEMENT */
				} else {
					insub = 1;
				}
			} else {
				insub = 1; /* on */
			}
			++p;
		} else 
		if (*p == ']') {
			if (!insub)
				/* VERY IMPORTANT - CHECK LAST SET INDEX */
				*q++ = *p; /* last index */
			insub = 0; /* off */
			++p;
		} else 
		if (insub == 1) {
			++p;
		} else {
			/* insub == 0 && x != '[' || ']' */
			*q++ = *p++;
		}
	}

	/* return string should be os_free()'ed.  */
	return ns;
}

static 
int __indexed_strcmp( s8 *path, s8 *s )
{
	s8 *p = __nonindex_string(path, 0); /* path has no set subscription index */
	s8 *x = __nonindex_string(s, 0);    /* calced path including subscription */
	int res;

	res = strcmp(p,x);
	if (!res) {
		/*  */
		JDBG("\t\t{\n\t\t%s: [%s] -> [%s] \n",__func__,path,p);
		JDBG("\t\t%s: [%s] -> [%s] \n\t\t}\n",__func__,s,x);
		/*  */
	}

	if (p) os_free(p);
	if (x) os_free(x);

	return res;
}

static 
int __indexed_strcmp2( s8 *path, s8 *s )
{
	s8 *p = __nonindex_string(path, 1); /* path has no set subscription index */
	s8 *x = __nonindex_string(s, 1);    /* calced path including subscription */
	int res;

	res = strcmp(p,x);
	if (!res) {
		/*  */
		JDBG("\t\t{\n\t\t%s: [%s] -> [%s] \n",__func__,path,p);
		JDBG("\t\t%s: [%s] -> [%s] \n\t\t}\n",__func__,s,x);
		/*  */
	}

	if (p) os_free(p);
	if (x) os_free(x);

	return res;
}

static 
int __setindex_strcmp( s8 *path, s8 *s )
{
	s8 *p = __nonindex_string(path, 1); /* path has no set subscription index */
	s8 *x = __nonindex_string(s,  1);   /* calced path including subscription */
	int res;

	res = strcmp(p,x);
	if (!res) {
		/*  */
		JDBG("\t\t{\n\t\t%s: [%s] -> [%s] \n",__func__,path,p);
		JDBG("\t\t%s: [%s] -> [%s] \n\t\t}\n",__func__,s,x);
		/*  */
	}

	if (p) os_free(p);
	if (x) os_free(x);

	return res;
}

static 
int __normal_strcmp( s8 *path, s8 *s )
{
	return strcmp( path, s );
}

static
int __make_set_block_path( s8 *s , int check )
{
	s8 *begin = s;
	s8 *end = s+strlen(s)-1;

	if (*end == ']') {
		int num = 0;
		while (1) {
			--end;
			if (!(*end != '[' && (end != begin) && (*end != '.')))
				break;
			if (!(*end >= '0' && *end <= '9'))
				return num;
			num = num*10 + (*end - '0');
		}
		if (*end == '[' && !check ) 
			*end = 0x0; /* input string is changed */
		return num;
	} else
		return -1;
}

static 
int __subpath_strcmp( s8 *path, s8 *s )
{
	s8 *p = os_strdup(path);
	s8 *x = os_strdup(s);       /* calced path including subscription */
	int res;
	s8 *n;

	/*
	 *  ex) 
	 *
	 *  path = CC.DD
	 *     s = AA.BB.CC.DD    --> path == s !!! 
	 *
	 */

	n = x + (strlen(x) - strlen(p));

	/* p ? path ?  */
	res = ( strcmp(n, p) == 0 ) ? 0 : -1; /* compare with non-indexed string */
	if (!res) {
		/*  */
		JDBG("\t\t{\n\t\t%s: [%s] -> [%s] \n",__func__,path,p);
		JDBG("\t\t%s: [%s] -> [%s] \n\t\t}\n",__func__,s,x);
		/*  */
	}

	if (p) os_free(p);
	if (x) os_free(x);

	return res;
}

static 
sjson_node_t * __sjson_search_subtree_internal( sjson_node_t * node, s8 *path, int (*compare)(s8 *,s8 *), sjson_pl_t ** root )
{
	s8 *p;
	sjson_node_t * xpath;

	if (!node || !path || !compare)
		return NULL;

	/* initial .. */
	xpath = NULL;

	if (node->type == TOKEN) {
		/* only TOKEN node */
		if (!node->value && 
				!__sjson_set_member( node, NULL, NULL )) {
			xpath = NULL;
			goto __continue_search_subtree;
		}

		/* dump search path */
		p = __sjson_path_name(node);
		if (!p) {
			xpath = NULL;
			goto __continue_search_subtree;
		} else {
	 		if ( !compare( path, p ) ) {
				sjson_pl_t * temp;
				xpath = node; /* result */
		
				/* found path list */	
				temp = (sjson_pl_t *)os_malloc(sizeof(sjson_pl_t));
				if (!temp)
					goto __continue_search_subtree;
				temp->path = os_strdup( p );
				temp->next = (*root);
				(*root) = temp;
			}
			os_free(p);
		}
	} 

  __continue_search_subtree:

	return (sjson_node_t *)((long)xpath | 
			(long)__sjson_search_subtree_internal( node->value , path , compare , root ) | 
			(long)__sjson_search_subtree_internal( node->bros  , path , compare , root ));
}

static 
sjson_node_t * __sjson_get_subtree( sjson_node_t * node, s8 *path, int (*compare)(s8 *,s8 *) )
{
	sjson_node_t * res;
	sjson_pl_t *proot = NULL, *pnode = NULL ;
	sjson_pl_t * pselected; /* selected from conflicts */

	res = __sjson_search_subtree_internal( node, path, compare, &pnode );

	if (pnode) {
		if (!pnode->next) {
			sjson_path_delete( pnode );
			return res;
		}
	}

	if (!pnode)
		return NULL;

	pselected = NULL;
	
	proot = pnode;
	if (pnode->next) {
		int cnt = 0;
		int len, clen;

		JDBG("[ ");
		while (pnode) {
			++cnt;
			JDBG("%s",(char *)pnode->path);
			pnode = sjson_path_next(pnode, 0);
			if (pnode)
				JDBG(", ");
		}
		JDBG("] - conflict %d \n", cnt );
	
		/* selecting one from conflicted list - 
		   an item with the smallest length */
		pnode = proot;
		clen = 10000;
		while (pnode) {
			len = strlen(pnode->path);
			if (len < clen) {
				pselected = pnode;
				clen = len;
			}
			pnode = pnode->next;
		}
	}

	/*
	 *
	 * conflict --> resolve conflicts by selecting an item with smallest length 
	 *
	 */
	if (pselected) {
		sjson_pl_t * rpnode = NULL;

		JDBG("%s : rescanning with %s \n",__func__,pselected->path);
		res = __sjson_search_subtree_internal( node, pselected->path , 
				__normal_strcmp , &rpnode );
		if (rpnode->next)
			JDBG("%s : Failed to non-conflict set\n",__func__);

		sjson_path_delete( rpnode );
		sjson_path_delete( proot );
		return res;
	}

	sjson_path_delete( proot );
	return NULL;
}

static
sjson_node_t * __sjson_find_subroot( sjson_node_t * node, s8 *path, int (*compare)(s8 *,s8 *), s8 **subpath, int setindex )
{
	s8 * spath;
	s8 * p;
	int sz;
	sjson_node_t * t = NULL;

	sz = (int)strlen(path);

	spath = (s8 *)os_malloc(sz+1);
	if (!spath)
		return NULL;

	strncpy(spath,path,sz);

	if (setindex != -1) {
		JDBG("%s: __sjson_get_subtree( %s - SETINDEX_STRCMP ) \n",__func__,spath);
		t = __sjson_get_subtree( node, spath, __setindex_strcmp );
		if (t)
			goto __subtree_found;
	}

	p = strrchr(spath,'.');
	do {
		*p = 0x0;
		JDBG("%s: __sjson_get_subtree( %s ) \n",__func__,spath);
		t = __sjson_get_subtree( node , spath , compare );
		p = strrchr(spath,'.');
	} while ( !t && p );

__subtree_found:

	if (subpath)
		(*subpath) = spath; /* subpath should be freed up */
	else
		os_free( spath ); /* should be cleaned up here */

	return t;
}

static 
sjson_node_t * __sjson_get_uptree( sjson_node_t * node ) 
{
	sjson_node_t * p = node;
	sjson_node_t * pp, * tok, * n, *nroot = NULL;
	/*sjson_node_t * end; */
	int lvl;

	if (!node)
		return NULL;

	nroot = NULL;
	lvl = node->lvl; /* current level */

	if (p->type != TOKEN)
		ERR("\t%s: weird~ beginning is not token\n",__func__);

	/* Why ?? --> Look at the reference point */
	p = p->prev; /* skipping TOKEN */

	pp = p;
	while ( p ) {
		tok = NULL;
		if (p->type == STMT) {
			if (multiple_node(pp) && 
					(p->value != pp)) {
				sjson_node_t *v;

				/*
			 	* STMT
			 	*   TOKN
			 	*   BLOCK ....
				*/
				v = p->value;
				tok = __sjson_new_node( v->type , v->lvl, os_strdup( symbol_name(v) ) );
				if (!tok) {
					return NULL;
				} 
			} else {
				p = p->prev; /* SKIP this node... */
				continue;
			}
		} 
		//JDBG("\t%s: (%s) \n",__func__,symbol_name(p));

		n = __sjson_new_node( p->type , 
				p->lvl , 
				os_strdup( symbol_name(p) ) );
		if (!n) 
			return NULL;

		//JDBG("\t%s: >node=%s:%s %-2d %-2d\n",__func__,__node_name(n),symbol_name(n),lvl,p->lvl);
		switch (p->type) {
		case BLOCK:
		case SET:
			#if 1
			if (lvl != p->lvl) 
				n->value = nroot;
			else 
				n->bros  = nroot;
				
			#else
			/* build up EMPTY SET node */
			if ((p->type == SET) && (p->bros)) {
				if (lvl == p->lvl) {
					sjson_node_t * trace = p->bros; /* set starting node */
					sjson_node_t * sroot;
					sjson_node_t * temp, * blk0, *blk1;

					sroot = NULL;
					while (trace->bros != nroot) {

						printf("\tSET<%s>\n",symbol_name(trace));
	
						/* previous nodes are exist ... */
						switch (trace->type) {
						case TOKEN:
							/* NULL TOKEN node */
							temp = __sjson_new_node( TOKEN, lvl, NULL );
							break;
						case BLOCK:
							/* NULL block node */
							blk0 = __sjson_new_node( BLOCK, lvl, os_strdup("BLKN0") );
							blk1 = __sjson_new_node( BLOCK, lvl+1, os_strdup("BLKN1") );
							blk0->value = blk1;
							temp = blk0;
							break;
						}

						if (!sroot) {
							sroot = temp;
						} else {
							end = sroot;
							while (end->bros) 
								end = end->bros;
							end->bros = temp;
						}
						if (!trace->bros)
							break;
						trace = trace->bros;
					}
					end = sroot;
					while (end->bros) 
						end = end->bros;
					end->bros = nroot;
					nroot = sroot;
				}
			}

			if (lvl != p->lvl) 
				n->value = nroot;
			else 
				n->bros  = nroot;
			#endif
			break;
		case STMT:
			if (tok) {
				n->value = tok;
				tok->value = nroot;
			} else {
				ERR("\t%s: ?what STMT %s\n",__func__,symbol_name(p));
			}
			break;
		}

		pp = p;
		lvl = p->lvl;
		nroot = n;
		p = p->prev;		
	}

	return nroot;
}

static 
s32 __inline__ prio_path( sjson_node_t *node )
{
	sjson_node_t *n;
	
	if ( __sjson_set_member( node, NULL, NULL ) )
		return 1;  /* PRIO node */

	if (node->value) {
		n = node->value;
		if (empty_node(n)) 
			return 1;
	}

	return 0;
}

static
s32 __sjson_list_path( sjson_node_t * node, sjson_pl_t **root )
{
	char *p;
	s32 xlen = 0;
	s32 v = 0, b = 0;
	sjson_pl_t *temp;

	if (!node)
		return 0;

	if (node->type == TOKEN) {
		/* only TOKEN node */
		if (!node->value && 
				!__sjson_set_member( node, NULL, NULL )) 
			return 0;

		/* dump search path */
		p = __sjson_path_name(node);
		if (!p) {
			return 0;
		} else {
	 		xlen = strlen(p);

			temp = (sjson_pl_t *)os_malloc( sizeof(sjson_pl_t) );
			if (!temp) {
				ERR("os_malloc(%d) failed\n", (int)sizeof(sjson_pl_t));
				os_free(p);
				return 0;
			}
			temp->path = os_strdup(p);
			temp->flag = (node->flag & F_DEL) ? F_DEL : 0;
			if (prio_path(node))
				temp->flag |= F_PRIO;
			temp->next = NULL;
			os_free(p);
			
			if ( (*root) ) {
				sjson_pl_t * trace = (*root);
				while (trace->next)
					trace = trace->next;
				trace->next = temp;	
			} else {
				(*root) = temp;
			}
		}
	} 

	v = __sjson_list_path( node->value , root ); /* value fist */
	b = __sjson_list_path( node->bros  , root ); /* brothers next */

	return v+b+xlen;
}

static void
__sjson_backward_rechain( sjson_node_t * node )
{
	sjson_node_t *t, *p;

	if (!node)
		return ;

	__sjson_backward_rechain( node->bros ); /* For easy control, brother scan is better to be called first */
	__sjson_backward_rechain( node->value );

	switch (node->type) {
	case BLOCK:
	case SET:
		p = node;
		t = node->bros;
		if (!t) {
			if (node->value) {
				t = node->value;
				t->prev = node; /* value node */
			}
		} else {
			while (t) {
				if (node->type == SET ) /* SET is different */
					t->prev = node;
				else
					t->prev = p;
				p = t;
				t = t->bros;
			}
		}
		if (node->type == BLOCK && 
				node->value) {
			/* exception case */
			p = node->value;
			p->prev = node;
		}
		break;
	case STMT:
		t = node->value;
		if (t) { /* only 2 item */
			t->prev = node;
			if (t->value) {
				p = t;
				t = t->value;
				t->prev  = node;
			}
		}
		break;
	}

}

/* cleaning up comma notation */
static void 
__sjson_clear_comma( sjson_node_t *node )
{
	sjson_node_t * p , * pbros;

	if (!node)
		return ;

	p = node->value;

	while (p) {
		if (!multiple_node(p)) {
			pbros = node->prev;
			if (!pbros) {
				ERR("\t%s: Xnode(%s)->prev NULL \n",__func__,__node_name(node));
				break;
			}
			if (!pbros->bros) {
				JDBG("\t%s: X%s p=%s->comma = 0\n",__func__,__node_name(p), symbol_name(p));
				p->flag &= ~F_COMMA;
			}
			break;
		}
		if (p->flag & F_COMMA) {
			if (!node->bros) {
				JDBG("\t%s: Z%s p=%s->comma = 0\n",__func__,__node_name(p), symbol_name(p));
				p->flag &= ~F_COMMA;
			}
		}
		p = p->value;
	}
}

static 
sjson_node_t * __sjson_search_ll_node( sjson_node_t * node )
{
	sjson_node_t * p;

	if (!node)
		return NULL;

	p = node->value;
	if (!p)
		return NULL;

	while (p->value) {
		p = p->value;
		if (!p) 
			return NULL;
		if (!multiple_node(p))
			break;
	}

	return p;
}

/* cleaning up comma notation */
static void 
__sjson_add_comma( sjson_node_t *node )
{
	sjson_node_t * p;

	p = __sjson_search_ll_node( node );
	if (!p)
		return;

	if (p->flag & F_COMMA)
		return ;
	p->flag |= F_COMMA;
}

static void
__sjson_level_assign( sjson_node_t * node , int lvl )
{
	int nlvl;

	if (!node)
		return ;

	nlvl = lvl;
	if (multiple_node(node)) {
		if (node->value) {
			if (multiple_node(node->value))
				nlvl = lvl+1;
		}
	}

	node->lvl = lvl;

	__sjson_level_assign( node->value , nlvl );
	__sjson_level_assign( node->bros  , lvl );
}

static 
sjson_node_t * __sjson_find_partial_root( sjson_node_t * cu, s8 * path )
{
	s8 * buff, *p;
	int sz;
	int done;
	sjson_node_t * trace, *prev, *final;

	if (!path || !cu)
		return NULL;

	sz = (int)strlen(path);

	buff = (s8 *)os_malloc( sz+1 );
	if (!buff)
		return NULL;

	memset(buff, 0, sz+1);
	strncpy(buff, path, sz);

	final = NULL;
	prev = trace = cu;
	done = 0;
	while ( !done && trace ) {
		p = strrchr(buff,'.');
		if (!p) {
			p = buff;
			done = 1;
		} else {
			*p = 0;
			p++; /* p points symbol name */
		}

		while( trace->type != STMT ) {
			trace = trace->prev; /* goes up */
			if (trace->type == STMT && 
					((trace->bros == prev) ||    /* CASE.1 - same level tokens in a block 
						-- TODO. What happens more than 2 brothers !!! ??? */
					 (trace->prev == prev->prev) /* CASE.2 - same level tokens in a set */
					 )) {
				/* in the middle of same level */
				trace = trace->prev;
			} 
		}
		prev = trace;

		if (!done)
			trace = trace->prev;
	}

	final = trace;

	os_free( buff );

	return final;
}

static 
int __sjson_count_visible_nodes( sjson_node_t *n )
{
	int cnt = 0;

	if (!n)
		return 0;

	if (n->type == TOKEN) {
		if (!(n->flag & F_DEL)) 
			cnt = 1;
	} else if (n->type == STMT) {
		if (n->value || n->bros)
			cnt = 1;
	}

	/*
	 *
	 * Just counting how many STMTs and TOKENs
	 *
	 */
	return cnt + __sjson_count_visible_nodes(n->bros) + __sjson_count_visible_nodes(n->value);
}

static __inline__
int __sjson_set_has_tokens( sjson_node_t *node )
{
	sjson_node_t * trace = node;

	while ( trace ) {
		if ( !((trace->type == TOKEN) && !(trace->value)) )
			return 0;
		trace = trace->bros;
	}

	return 1;
}

static 
int __sjson_purge_json_internal( sjson_node_t *r )
{
	int cnt = 0;
	sjson_node_t * v, *p;

	/*
 	*
	* Navigate -> navigate -> nAvigate .... = Very Very EXPENSIVE operation 
	*
 	*/

	if (!r)
		return 0;

	switch (r->type) {
	case SET:
		if (!r->bros)
			goto __next_purge;
	case BLOCK:
		if (!r->bros)
			goto __next_purge;

		p = r->bros;

		/* SET exception case */
		if (r->type == SET) {
			if (__sjson_set_has_tokens(r->bros)) {
				sjson_node_t * trace = r;
				sjson_node_t * p, * prev;

				prev = trace;
				while ( trace ) {
					if (trace->flag & F_DEL) {
						prev->bros = trace->bros;
						p = trace;
					} else 	{
						p = NULL;
						prev = trace;
					}
					trace = trace->bros;
					if (p) {
						JDBG("%s: EMPTY SET TOKEN - %s\n",__func__,symbol_name(p));
						__sjson_free_node(p);
						++cnt;
					}
				}
			}
			goto __next_purge;
		}

		if (!p->value)
			goto __next_purge;

		if (!__sjson_count_visible_nodes(p->value)) {
			r->bros = p->bros;
			p->bros = NULL;
			__sjson_clear_comma( r );
			JDBG("%s: EMPTY BLOCK - %s\n",__func__,symbol_name(p));
			__sjson_free( p );
			cnt = 1;
		}
		break;

	case STMT:
		/*
		 * STMT : only key but without no value 
		 */
		/* siblings... */
		v = r->value;
		if (v && (v->type == TOKEN)) {
			v = v->value;	
			if (multiple_node(v)) {
				if (!__sjson_count_visible_nodes(v)) {
					JDBG("%s: EMPTY STMT - %s\n",__func__,symbol_name(v));
					v = r->value;
					__sjson_free( v );	 
					r->value = NULL;
					cnt = 1; 
				}
			}
		}
		/* fall through */

	case TOKEN:
		if (r->bros) {
			if (!__sjson_count_visible_nodes(r->bros)) {
				/* Key node of BLOCK/STMT */
				JDBG("%s: EMPTY BROS (TOKEN/STMT) - %s\n",__func__, symbol_name(r->bros));
				__sjson_free(r->bros);
				r->bros = NULL;
				cnt += 1;
			}
		}
		break;
	}

  __next_purge:
	return cnt + __sjson_purge_json_internal(r->bros) + __sjson_purge_json_internal(r->value);
}

static 
int __sjson_delete_json_subtree_internal( sjson_node_t * n )
{
	sjson_node_t * cst;
	sjson_node_t * p , * temp;

	/* correspond statement */
	cst = n->prev;
	if (!cst) {
		ERR("No correspond statement \n");
		return -1;
	}

	if (cst->value != n) {
		/* special case... SET member */
		if ((cst->type == SET) && 
				(n->type == TOKEN)) {
			/* SET cases... */
			temp = p = cst;
			while (p) {
				if (p == n) 
					break; /* n is the direct member of a set cst */
				temp = p;
				p = p->bros;
			}
			if (!p) {
				ERR("%s: %s is not found in a set %s\n",__func__, symbol_name(n), symbol_name(cst));
				return 0;
			}
			/* temp -> prev of p */
			p->flag |= F_DEL; /* deleted !! */
			JDBG("%s: ** FREE (%s)\n", __func__, symbol_name(p));
		} else
			ERR("statement error (cst=%s, n=%s)\n", symbol_name(cst), symbol_name(n));
		return 0;
	}

	p = cst->prev;
	if (!p)
		return 0;

	if (p->type == BLOCK) {

		p->bros = cst->bros;
		temp = cst->bros;
		if (temp)
			temp->prev = p; /* down -> up */
		cst->bros = NULL;
		JDBG("%s: ^^ FREE (%s) \n",__func__,symbol_name(cst));  /* STMT !! */
		__sjson_free( cst );

		if (empty_node(p)) {
			temp = p->prev;
			if (!temp) {
				JDBG("%s: null p->prev(%s) !! strange!! \n",__func__,symbol_name(p));
				return -1;
			}
			if (temp->value == p) {
				p = temp;
				JDBG("%s: %ss START \n",__func__,symbol_name(p));
				temp = temp->prev; /* goes up again */
				if (!temp) {
					JDBG("%s: null p->prev(%s) -> prev !! strange!! \n",__func__,symbol_name(p));
					return -1;
				}
				JDBG("%s: %s CHECK \n",__func__,symbol_name(temp));
				while (temp->bros != p) {
					JDBG("%s: %s->bros is not %s \n",__func__,symbol_name(temp),symbol_name(p));
					temp = temp->bros;
					if (!temp)
						break;
				}
				if (!temp) {
					JDBG("%s: failed in searching temp !! strange!! \n",__func__);
					return -1;
				}
				/* 
				 *
				 * BLOCK node will be deleted after doing "purge" command. 
				 * To remain block node, it is very effective to maintain the index of SET !!
				 * Block is not deleted immediately. 
				 *
				 */
				p->flag |= F_DEL; /* deleted !! */
				JDBG("%s: ++ FREE (%s) \n", __func__, symbol_name(p));
			}
			p = temp;
		}
		__sjson_clear_comma( p );
		return 0;
	}

	/* STMT */
	JDBG("\t%s: %s->bros = %s->bros \n", __func__,symbol_name(p),symbol_name(cst));
	p->bros = cst->bros;    /* up -> down */
	temp = cst->bros;
	if (temp)
		temp->prev = p; /* down -> up */

	p = p->value;  /* update p pointer */
	if (!p) {
		ERR("%s - STMT error\n",__func__);
	}
	cst->bros = NULL;

	/*
	 * THIS IS VERY IMPORTANT - ONE STEP MOTHER NODE - BROTHER NODE CHECK !!
	 */
	{
		temp = p->prev;
		if (!temp)
			return 0;
	}
	if (!temp->bros)
		__sjson_clear_comma( p );

	JDBG("%s - << FREE (%s)\n",__func__,symbol_name(cst));

	__sjson_free( cst );
	return 0;
}

static
sjson_node_t * __sjson_delete_json_internal( sjson_node_t *r, s8 *path , int (*compare)(s8 *, s8 *))
{
	sjson_node_t * n;

	if (!r || !path || !compare )
		return NULL;

	n = __sjson_get_subtree( r, path, compare ); /* sub tree */
	if (!n)
		return NULL;

	__sjson_delete_json_subtree_internal( n );
	return r;
}

static
sjson_node_t * __sjson_get_set_value( sjson_node_t * p )
{
	sjson_node_t * found = NULL;

	if (!p)
		return NULL;

	if (p->type == TOKEN && empty_node(p))
		found = p;

	return (sjson_node_t *)((long)found + 
			(long)__sjson_get_set_value(p->value) + (long)__sjson_get_set_value(p->bros));
}

static 
void __sjson_precalc_setindex( sjson_node_t * node )
{
	if (!node)
		return ;

	if ((node->type == SET) && 
			!(node->value)  &&
			node->bros) {
		int cnt = 0;
		sjson_node_t * trace;

		trace = node->bros;
		while (trace) {
			++cnt;
			trace = trace->bros;
		}
		if (cnt > 0) {
			sjson_update_set_count( node , cnt );
			JDBG("%s: update_set_count( %s , %d/%d ) \n",__func__,symbol_name(node),cnt,sjson_get_setcount(node));
		}
	}


	__sjson_precalc_setindex( node->value );
	__sjson_precalc_setindex( node->bros  );
}

/* 
 *
 * ---------- G L O B A L      F U N C T I O M S ------------------- 
 *
 */

void sjson_table_init( sjson_table_t *tbl )
{
	int i;

	if (!tbl)
		return;

	for (i = 0; i < MAXL; i++) {
		tbl->root[ i ] = NULL;
		tbl->last[ i ] = NULL;
	}
	tbl->lvl = -1;
}

sjson_node_t * sjson_copy( sjson_node_t * node )
{
	if (!node) 
		return NULL;
	else {
		sjson_node_t * temp;

		temp = __sjson_new_node( node->type , node->lvl, os_strdup( symbol_name(node) ) );
		if (!temp) 
			return NULL;

		temp->flag  = node->flag;
		temp->prev  = NULL; /* should be rechained by __sjson_backward_rechain */
		temp->value = sjson_copy( node->value );
		temp->bros  = sjson_copy( node->bros  );
		return temp;
	}
}

int sjson_parse( sjson_table_t *tbl, u8 *mem )
{
	u8 *sp = mem; /* scanner pointer */
	enum{
		stBLK = 100,
		stSET,
		stVAL,
	};
	#define STATEDEPTH 256
	s8  state[STATEDEPTH];
	s32 state_top = -1;
	#define current_state	state[ state_top ]
	#define push_state(x)	do{ state[ ++state_top ] = (x); }while(0)
	#define pop_state(x)	do{ --state_top; }while(0)
	#define TOKENSZ 4096
	//static s8  token[TOKENSZ];
	s8 *token;
	s32 tokenp;
	#define DIGIT(x)		(((x)>='0') && ((x)<='9'))
	#define ALPHA(x)		((((x)>='a') && ((x)<='z')) || (((x)>='A') && ((x)<='Z')))
	#define ALLOWED_CHARS(x)	(\
					((x) == ' ') || ((x) == '_') || ((x) == '+') || ((x) == '-') || \
					((x) == '.') || ((x) == '<') || ((x) == '>') || \
					((x) == '!') || ((x) == '@') || ((x) == '#') || ((x) == '$') || \
					((x) == '%') || ((x) == '^') || ((x) == '&') || ((x) == '*') || \
					((x) == '(') || ((x) == ')') || ((x) == '=') || ((x) == '~')    \
					)
	int blkn = 0; /* block     suffix number - symbol */
	int setn = 0; /* set       suffix number - symbol */
	int stmn = 0; /* statement suffix number - symbol */

	sjson_table_init( tbl );

	// allocating token buffer 
	token = (s8 *)os_malloc(TOKENSZ);
	if (!token)
		return -1;

	memset(state, 0, sizeof(s8)*STATEDEPTH);

	while (*sp) {

		switch (*sp) {
		case '{':
			push_state( stBLK );
			__sjson_add_node( tbl, BLOCK, state_top, __sjson_snm(BLOCK,&blkn) );
			JDBG("node  %d @{@ \n",state_top);
			break;
		case '}':
			JDBG("node  %d @}@ \n",state_top);
			__sjson_add_node( tbl, STMT, state_top, __sjson_snm(STMT,&stmn) );
			pop_state();
			__sjson_add_node( tbl, BLOCK, state_top, __sjson_snm(BLOCK,&blkn) );
			break;
		case '[':
			push_state( stSET );
			__sjson_add_node( tbl, SET, state_top, __sjson_snm(SET,&setn) );
			JDBG("node  %d @[@ \n",state_top);
			break;
		case ']':
			JDBG("node  %d @]@ \n",state_top);
			__sjson_add_node( tbl, STMT, state_top, __sjson_snm(STMT,&stmn) );
			pop_state();
			__sjson_add_node( tbl, SET, state_top, __sjson_snm(SET,&setn) );
			break;

		case '0'...'9':  /* number */
		case 'a'...'z':  /* identifers */
		case 'A'...'Z':  /* --+        */
			tokenp = 0;
			memset(token,0,TOKENSZ);
			while (DIGIT(*sp) || ALPHA(*sp) || ALLOWED_CHARS(*sp)) {
				if (tokenp >= TOKENSZ) {
					ERR("Too long string\n");
					goto parse_error;
				}
				token[ tokenp++ ] = *sp++;
			}	
			token[ tokenp++ ] = 0x0;
			--sp; /* retreat */

			JDBG("token %d #%s# \n", state_top,token);
			__sjson_add_node( tbl, TOKEN , state_top, os_strdup(token) );
			break;
		case '"':
			tokenp = 0;
			memset(token,0,TOKENSZ);
			while (*(++sp) != '"') {
				if (*sp == '\\')
					++sp;
				if (tokenp >= TOKENSZ) {
					ERR("Too long string\n");
					goto parse_error;
				}
				token[ tokenp++ ] = *sp; 
			}
			token[ tokenp ] = 0x0;

			JDBG("token %d #%s# \n", state_top,token);
			__sjson_add_node( tbl, TOKEN , state_top, os_strdup(token) );
			break;
		case ':':
			break;
		case ',':
			__sjson_add_node( tbl, STMT,  state_top, __sjson_snm(STMT,&stmn) );
			__sjson_add_node( tbl, COMMA, state_top, NULL ); /* Comma processing... */
			break;
		default:
			break;
		}
		++sp; /* ahead.. */
	}

	os_free( token );
	return 0;

  parse_error:
	if (token)
		os_free( token );
	return -1;
}

int sjson_print_tree( sjson_table_t * tbl, s8 *buff, s32 size )
{
	if (!tbl)
		return 0;

	if (!tbl->root[0])
		return 0;

	return __sjson_tree_print_internal( tbl->root[0], 0, __sjson_max_depth( tbl->root[0], 0), buff, size );
}

int sjson_print_json( sjson_table_t *tbl , s8 *buff, s32 size )
{
	int len = 0;

	if (!buff || (size <= 0))
		return 0;

	len += __sjson_json_print_internal( tbl->root[0] , buff+len , size-len , 1 );
	return len;
}

int sjson_print_xml( sjson_table_t *tbl , s8 *buff, s32 size )
{
	int len = 0, tlen = 0;

	if (!buff || (size <= 0))
		return 0;

	/* Global variable only for XML printout use !! */
	/* CLUMSY !! */
	json_xml_setelm = 0;
	json_xml_setlvl = 0;

	tlen  = snprintf((char *)buff+tlen,size-tlen,"<?xml version=\"1.0.0.0\" encoding=\"utf-8\"?>\n"); 
	tlen += snprintf((char *)buff+tlen,size-tlen,"<%s>\n",JSON_XML_ROOT);
	len   = __sjson_xml_print_internal( tbl->root[0] , buff+tlen , size-tlen , 0 );
	tlen += snprintf((char *)buff+(len+tlen),size-(len+tlen),"</%s>\n",JSON_XML_ROOT);

	return len;
}

int sjson_free( sjson_table_t *tbl )
{
        int i;

	if (!tbl)
		return 0;

	/* tree cleanup */
	__sjson_free( tbl->root[0] );

	tbl->root[0] = NULL; /* nullifying */
	tbl->last[0] = NULL; /* nullitying */

	/* check !! */
	for (i = 0; i < MAXL; i++) {
		if (tbl->root[i]) 
			return -1;
	}

	return 0;
}

sjson_pl_t * sjson_path( sjson_table_t *tbl )
{
	sjson_pl_t *proot = NULL;

	__sjson_list_path( tbl->root[0], &proot );

	return proot;
}

sjson_pl_t * sjson_path_next( sjson_pl_t * node , int prio )
{
	sjson_pl_t *trace = node;

	if (!node)
		return NULL;

	if (!prio)
		return node->next;

	do {
		trace = trace->next;
		if (!trace)
			break;
		if (trace->flag & F_PRIO)
			break;
	} while ( 1 );

	return trace;
}

sjson_node_t * sjson_find_json( sjson_node_t *r , s8 *path , int mode )
{
	sjson_node_t * n, * x, * ptree;
	sjson_node_t * h, *b; /* header block */
	s8 * pin_sn = NULL;

	if (!r || !path)
		return NULL;

	/* subtree */
	x = __sjson_get_subtree( r , path , __normal_strcmp ) ;
	if (!x) 
		return NULL;

	if (x->value == NULL) {
		/*
		 * special case - [JSON] find 0 aa.bb.cx[2] 
		 *
		 */
		sjson_node_t *f = x; /* found */

		/* exception case - SET */
		do {
			x = x->prev;
			if (!x)
				break;
			if (x->type == STMT)
				break;
		} while (x);
		if (!x) {
			ERR("%s: searching STMT node failed from %s \n",__func__,symbol_name(f));
			return NULL;
		}
		x = x->value; /* TOKEN */
		if (x->type != TOKEN) { /* validity check */
			ERR("%s: failed at path=(%s) node=%s \n",__func__,path,symbol_name(x));
			return NULL;
		}
		pin_sn = os_strdup( symbol_name(f) ); /* symbol name */
		if (!pin_sn) {
			ERR("%s: os_strdup( %s ) failed\n",__func__, symbol_name(f));
			return NULL;
		}
	}

	ptree = NULL;

	if (mode == 0) {
		/*
		 * mode = 0 : full path name is preserved
		 *      = 1 : just sub tree is extracted
		 *
		 */
		/* full path name is preserved */
		ptree = __sjson_get_uptree( x );	/* parent tree */
		if (!ptree)
			return NULL;
	}

	/* copy subtree */
	n = sjson_copy( x );
	if (!n) {
		if (ptree) __sjson_free( ptree );
		return NULL;
	}

	if (pin_sn) { /* exceptional case */
		sjson_node_t *w, *nt;

		h = n; /* n == STMT */
		h = h->value;
		if (!h) {
			__sjson_free( n ); /* failed */
			return NULL;
		}
		while ((h->type == SET) && (h->value))
			h = h->value;
		
		w = h->bros;                                    /* will be deleted */
		nt = __sjson_new_node( TOKEN, 0, pin_sn );      /* new node is created */
		if (!nt) {
			ERR("%s: sjson_new_node( TOKEN %s ) failed \n",__func__, pin_sn);
			os_free(pin_sn);
			__sjson_free( n ); 
			return NULL;
		}
		nt->lvl = w->lvl; /* level */
		
		h->bros = nt;                                   /* replaced */
		__sjson_free( w );                              /* deleted */
	}

	if (ptree) {
		x= __sjson_new_node( STMT , 0, os_strdup( "STM0" ) ); /* next STMT node */
		if (!x)
			return NULL;

		/* IMPORTANT : down to the bottom of parent tree */
		h = ptree;
		while ((h->value) || (h->bros)) {
			if (h->value)
				h = h->value;
			if (h->bros)
				h = h->bros;
			if (!h)
				break;
		}

		/* appending a subtree to higher level parent tree */
		h->bros = x;
		x->lvl = h->lvl;
		x->value = n;

		/* b is the high level subtree */
		b = ptree;
	} else {
	
		/*
	 	* Adding headers..
	 	*/
		b= __sjson_new_node( BLOCK, 0, os_strdup( "BLK0" ) ); /* creating head node */
		if (!b) {
			__sjson_free( n );
			return NULL;
		}

		h= __sjson_new_node( STMT , 0, os_strdup( "STM0" ) ); /* next STMT node */
		if (!h) {
			__sjson_free( n );
			__sjson_free_node( n );
			return NULL;
		}

		b->bros = h;
		h->value = n;
	}

	__sjson_backward_rechain( b ); /* rechaining */
	__sjson_clear_comma( n );
	__sjson_level_assign( b, 0 ); /* reassign indentation level for beautiful display .... */

	return b;
}

sjson_node_t * sjson_delete_json( sjson_node_t *r, s8 *path , int mode )
{
	int (*fn)(s8 *, s8 *) = NULL;

	switch (mode) {
	case 0 : 
		fn = __normal_strcmp;
		break;
	case 1 :
		fn = __indexed_strcmp;
		break;
	}
	return __sjson_delete_json_internal( r, path, fn );
}

int sjson_purge_json( sjson_node_t *r )
{
	return __sjson_purge_json_internal( r );
}

void sjson_path_delete( sjson_pl_t * list )
{
	sjson_pl_t *q, *p = list;

	while (p) {
		q = p;
		p = p->next;
		os_free(q->path);
		os_free(q);
	}
}

int sjson_precalc_setmember( sjson_table_t * tbl )
{
	__sjson_precalc_setindex( tbl->root[ 0 ] );
	return 0;
}

sjson_node_t * sjson_add_json( sjson_node_t * n, sjson_node_t * a, s8 *spath )
{
	sjson_node_t * child, * parent;
	sjson_node_t * head,  * trace;
	s8 *path = NULL;
	s8 *bpath; /* BLOCK PATH */
	s8 *nsubpath;
	int index; /* Last set index */
	int xindex = -1; /* SET ELEMENT */
	#ifdef VERBOSE_ADD
	#define TMPSZ (16*1024)
	static char buff[TMPSZ];
	#endif 

	if (!n || !a)
		return NULL;

	head = trace = NULL;

	JDBG("%s: [%s] = \n", __func__,spath);

	/*
	 * A.B.C.D[1]   - D is actually "set".
	 *   bpath = "A.B.C.D"
	 *   index = 1
	 *
	 */
	bpath = os_strdup( spath );
	index = __make_set_block_path( bpath , 0 );
	if ( index == -1 ) {
		os_free( bpath );
		bpath = spath; /* original path */
	} 
	JDBG("%s: [%s] %d Finally \n",__func__, bpath, index);
	parent = __sjson_find_subroot( n, bpath, __indexed_strcmp, &path, index );
	if (parent) {

		if (!path) {
			ERR("%s: null path - fatal error\n",__func__);
			return n;
		}

		/* spath = "address.AA.BB.CC.Todd.Home" 
		 * path = "address.AA.BB.CC"
		 *
		 * spat + path + 1 = "Todd.Home"
		 */
		nsubpath = spath + strlen(path) + 1;

		JDBG("%s: [%s] + [%s] : \n", __func__, path , nsubpath); /* parent is SET */
		{
			xindex = __make_set_block_path( path , 1 );	
			if (xindex != -1) 
				JDBG("%s: SET-PARENT PATH=<%s> INDEX=<%d>\n",__func__,path,xindex);
		}

		child = __sjson_get_subtree( a, nsubpath, __subpath_strcmp );
		if (!child) {
			sjson_node_t * elem_child;
			sjson_node_t * v, * pchild;

			/* SET ELEMENT CHILD */
			elem_child = __sjson_get_subtree( a, path , __indexed_strcmp2 );
			if ( !elem_child ) {
				ERR(" Weird !! %s not SUBTREE !!\n", path );
				os_free(path);
				return n;
			}

			child = __sjson_get_set_value( elem_child );
			if (!child) {
				ERR(" Weird !! %s node not found !!\n", path );
				os_free(path);
				return n;
			}
	
			pchild = sjson_copy( child );

			v = parent->value; /* SET */
  			if (!v) {
				JDBG("%s: PARENT->VALUE = NULL\n",__func__);
				__sjson_free(pchild);
				goto __error_set_elem_add;
			}
			v = v->value;
	
			if (!v) {
				JDBG("%s: PARENT->VALUE->VALUE = NULL\n",__func__);
				__sjson_free(pchild);
				goto __error_set_elem_add;
			}
			v = v->bros;
		
			if (!v) {
				JDBG("%s: PARENT->VALUE->VALUE->BROS = NULL\n",__func__);
				__sjson_free(pchild);
				goto __error_set_elem_add;
			}

			while (v->bros)
				v = v->bros;

			v->bros = pchild;
			pchild->prev = v->prev;
			pchild->value = NULL;
			__sjson_add_comma( parent ); /* comma processing */

		__error_set_elem_add:
			os_free(path);
			return n;
		}

		/* partial root */
		child = __sjson_find_partial_root( child, nsubpath ); 
	} else {
		parent = n; /* Beginning of an original tree */
		child = a;  /* full tree */
	}
	JDBG("%s: CHILD=%s PARENT=%s \n",__func__, symbol_name(child), symbol_name(parent));

	os_free(path); /* deleing path */

	#ifdef VERBOSE_ADD
	memset(buff,0,TMPSZ);
	__sjson_tree_print_internal( child, 0, 9, buff, TMPSZ ); 
	printf("CHILD(%s)\n",buff);
	printf("\n+++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	#endif

	trace = parent;

	if (parent == n) {

		JDBG("%s: PARENT(%s) = N(%s) \n",__func__,symbol_name(parent),symbol_name(n));

		/*
		 * SPECIAL CASE -- child tree is appended on the root. 
		 */
		while (trace->bros) 
			trace = trace->bros;

		child = sjson_copy( child->bros /* STMT */ );
		trace->bros = child; /* everything... */
		child->prev = trace;
	
		head = child;
		head->lvl = 0;

	} else {
		sjson_node_t * prev;
		int cnt;
		int premember_count;
		sjson_node_t * blkx0, * blki0;

	__add_rescan:
		/* skipping BLOCK/SET ... */
		do {
			prev = trace;
			trace = trace->value;
		} while (trace->value && multiple_node(trace));

		if (!trace->bros) {
			sjson_node_t * set0, * set1;

			/* sjson_node_t * blk0, * blk1; */

			/* single node */
			/* we need to add up new block */

			JDBG("%s: SINGLE NODE.... %s %s\n",__func__, symbol_name(trace),symbol_name(prev));

			/*
	 		* Adding headers..
	 		*/
			set0 = __sjson_new_node( SET, 0, os_strdup( "SETX0" ) ); /* creating head node */
			if (!set0)
				return n;

			set1 = __sjson_new_node( SET, 0, os_strdup( "SETI0" ) ); /* creating head node */
			if (!set1) {
				__sjson_free_node(set0);
				return n;
			}

			/*
			 *
			 *  "<TOKEN> HH -> <TOKEN> HELLO"  + INPUT BLOCK
			 *
			 *     should be transformed as ... trace-> <TOKEN> HELLO 
			 *
			 *  "<TOKEN> HH -> 
			 *     <SET> SET0 -> 
			 *       <SET> SET1 -> 
			 *          <TOKEN> HELLO ->
			 *          <BLOCK> BLK4 -> 
			 *            <BLOCK> BLK5 -> INPUT BLOCK "
			 */

			/* link ... */
			set0->value  = set1;
			set0->bros   = NULL;
			set0->prev   = prev->prev;

			set1->value  = NULL;
			set1->bros   = trace;
			set1->prev   = set0;
			set1->flag   = trace->flag; /* comma processing */

			trace->prev  = set1;
			prev->value  = set0;

			trace->bros  = NULL;

			/* update level */	
			set0->lvl = prev->lvl;
			set1->lvl = set0->lvl+1;
			trace->lvl = set1->lvl;
		} 

	       	premember_count = sjson_get_setcount(trace);
		JDBG("%s: ARRIVED AT NODE<%s> <%s> <%d> \n",__func__, symbol_name(trace),symbol_name(child),premember_count);

		cnt = 0;
		while (trace) {
			/*
			 *  "A": {
			 *  	"B": { ... },
			 *  	"C": { ... },
			 *  	"D": { ... }
			 *  };
			 *
			 *  + 
			 *
			 */
			if (trace->type == STMT) {
				if (!strcmp(symbol_name(trace->value),
						symbol_name(child->value))) {
					sjson_node_t * pchild; /* temporary pointer */

					trace = trace->value;
		
					parent = __sjson_search_ll_node( trace );
					pchild =  __sjson_search_ll_node( child ); 

					if (parent->type == TOKEN && 
							pchild->type == TOKEN) {

						/* STMT --> TOKEN */
						trace = trace->prev; /* STMT */
						__sjson_free( trace );
						trace = prev;
						break;

					} else {	
						if (!pchild) {
							ERR("\tNext child is NULL!!!\n");
							break;
						}

						if (pchild->bros) {
							child = pchild->bros;
						} else {
							child = pchild;
						}
						goto __add_rescan;
					}
				}
			}
			prev = trace;
			trace = trace->bros; /* end of current level */
			if (trace) {
				++cnt;
				JDBG("%s: %d SCAN NODE %s \n", __func__,cnt,symbol_name(trace));
			}
		}

		if (xindex != -1)
			JDBG("%s: XINDEX=%d+%d COUNT=%d <%s>\n",__func__,xindex,premember_count,cnt,
				(xindex+premember_count) == cnt? "LOCATED !!":"SET INDEX - BLOCK MISMATCH");

		if (!trace)
			trace = prev;

		JDBG("%s: FINAL TRACE=%s\n", __func__, symbol_name(trace) );

		blkx0 = blki0 = NULL;

		if ((multiple_node(trace) ||
				__sjson_set_member(trace, NULL, NULL))) {

			if ((xindex != -1) && 
					( (xindex+premember_count) != cnt)) {
				JDBG("%s: TRACE COUNT %d/(%d+%d) \n",__func__,cnt,xindex+1,premember_count);

				/* SET INDEX - BLOCK MISMATCH */
				sjson_node_t *v;
				v = trace->value;
				if (v) {
					v = v->bros;
					if (v && (v->type == STMT)) {
						trace = v; /* move trace */
						JDBG("%s: TRACE MOVE TO %s \n",__func__,symbol_name(trace));
					}
				}
			} else {
				JDBG("%s: BLOCK ATTACH %d/(%d+%d) \n",__func__,cnt,xindex,premember_count);

				/*
 				* Adding headers..
 				*/
				blkx0 =  __sjson_new_node( BLOCK, 0, os_strdup( "BLKX0" ) ); /* creating head node */
				if (!blkx0)
					return n;

				blki0 = __sjson_new_node( BLOCK, 0, os_strdup( "BLKI0" ) ); /* creating head node */
				if (!blki0) {
					__sjson_free_node(blkx0);
					return n;
				}

				/* Update level */
				blkx0->lvl = trace->lvl; 
				blki0->lvl = blkx0->lvl+1;

				blkx0->value = blki0;
				blki0->bros  = child;

				JDBG("%s: CHILD=%s -> %s \n", __func__, symbol_name(child), symbol_name(blkx0));
				child = blkx0;  

				JDBG("%s: BLOCK NODES PREPENDED %s %s %s %s \n",
					__func__, symbol_name(trace) , symbol_name(trace->value), symbol_name(trace->bros), symbol_name(trace->prev));
			}
		}

		JDBG("%s: CHILD copy( child=%s <%s> ) \n",__func__,symbol_name(child),__node_name(child));

		sjson_node_t * pchild = child;

		child = sjson_copy( pchild ); /* !! MEMORY LEAK !!! */

		#ifdef VERBOSE_ADD
		if (pchild) {
			memset(buff,0,TMPSZ);
			__sjson_tree_print_internal( pchild, 0, 9, buff, TMPSZ ); 
			printf("\n#########\n\"%s\"############\n", buff); 
		}
		#endif

		JDBG("%s: CHILD=%s \n",__func__, __node_name(child));

		trace->bros = child; /* everything... */
		child->prev = trace;

		if (blkx0)
			__sjson_free_node(blkx0);

		if (blki0)
			__sjson_free_node(blki0);

	}

	__sjson_backward_rechain( parent ); /* backward rechaining */
	__sjson_add_comma( trace );         /* comma processing */
	__sjson_level_assign( n, 0 );       /* reassign indentation level for beautiful display .... */

	#ifdef VERBOSE_ADD
	memset(buff,0,TMPSZ);
	__sjson_tree_print_internal( parent, 0, 9, buff, TMPSZ ); 
	printf("\n>>>>>>\n\"%s\"<<<<<<<<\n", buff); 
	#endif

	return n;
}

#ifndef EXTRA_WORKAROUND
/*
 * Toolkit function 
 */
void dump_mem_block( void *blk )
{
	sjson_node_t * node = (sjson_node_t *)blk;

	if (node) {
		switch( node->type ) {
		case BLOCK:
			JDBG("<BLOCK> %s \n",symbol_name(node));
			break;
		case TOKEN:
			JDBG("<TOKEN> %s \n",symbol_name(node));
			break;
		case STMT:
			JDBG("<STMT> %s \n",symbol_name(node));
			break;
		default:
			JDBG("<%s>\n",(char *)blk);
			break;
		}
	}
}
#endif
