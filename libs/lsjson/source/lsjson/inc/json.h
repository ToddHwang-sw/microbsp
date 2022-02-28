#ifndef __JSON_LIBRARY_HEADER__
#define __JSON_LIBRARY_HEADER__

#include "common.h"

extern u32 json_dbg;

/* ---------- STRUCTURE ------------------- */
typedef struct __sjson_node{
	#define NODE	0x01	
	#define TOKEN	0x02
	#define BLOCK	0x04
	#define SET	0x08
	#define STMT	0x10
	#define COMMA	0x20
	u32 type;	/* SET or SINGLE */
	s8 *symbol;
	s32 lvl; 	/* node level */

	#define F_COMMA	0x1
	#define F_DEL	0x2
	#define F_SIDX_MASK 0xFC
	#define sjson_set_setcount(a)		(((u8)(a) & (((F_SIDX_MASK) >> 2)&0xFF)) << 2)
	#define sjson_get_setcount(n)		(((u8)((n)->flag) & F_SIDX_MASK) >> 2)
	#define sjson_update_set_count(n,a)	{  \
				(n)->flag &= ~(u32)F_SIDX_MASK; \
				(n)->flag |= (sjson_set_setcount(a) & F_SIDX_MASK); \
				}
	u32 flag;	/* flag */
	struct __sjson_node *value;	/* (u8 *) or (sjson_node_t *) */
	struct __sjson_node *bros; 	/* brother */
	struct __sjson_node *prev;	/* previous bros */
}__attribute__((aligned)) sjson_node_t;

typedef struct __sjson_table {
	/* Max level is 24 */
	#define MAXL 24 
	s32 lvl;  /* block level */
	sjson_node_t *root[ MAXL ]; /* root node at each level */
	sjson_node_t *last[ MAXL ]; /* last node at each level */
}__attribute__((aligned)) sjson_table_t;

/* APIs... */
extern void sjson_table_init( sjson_table_t * );
extern int sjson_parse( sjson_table_t *, u8 * );
extern int sjson_print_tree( sjson_table_t *, s8 *, s32 );
extern int sjson_print_json( sjson_table_t *, s8 *, s32 );
extern int sjson_print_xml( sjson_table_t *, s8 *, s32 );
extern int sjson_free( sjson_table_t *);
extern int sjson_precalc_setmember( sjson_table_t * );
extern sjson_node_t * sjson_copy( sjson_node_t * node );
extern sjson_node_t * sjson_add_json( sjson_node_t *, sjson_node_t *, s8 * );
extern sjson_node_t * sjson_find_json( sjson_node_t *, s8 * , int );
extern sjson_node_t * sjson_delete_json( sjson_node_t *, s8 *, int );
extern int sjson_purge_json( sjson_node_t * );

/* Path list ... */
typedef struct __plist {
	s8 *path;
	#define sjson_path_prio(n)	(((n)->flag & F_PRIO) == F_PRIO)
	#define sjson_path_deleted(n)	(((n)->flag & F_DEL)  == F_DEL)
	#define F_PRIO 0x4
	s8 flag;
	struct __plist *next;
} __attribute__((aligned)) sjson_pl_t;

/* Path APIs... */
extern sjson_pl_t *	sjson_path( sjson_table_t * );
extern sjson_pl_t *	sjson_path_next( sjson_pl_t * , int );
extern void 		sjson_path_delete( sjson_pl_t *pl );

/* 
 * No "[ xxxx ]" concept inside of XML ...
 */
#define JSON_XML_SET_NAME	"elem"
#define JSON_XML_ROOT		"root"
#define JSON_PATH_DELIM		"."
#define JSON_PATH_LIST_DELIM	"\n"

#endif /* __JSON_LIBRARY_HEADER__ */
