#ifndef __PPS_LIST_HEADER__
#define __PPS_LIST_HEADER__

#include "sig.h"

#ifdef __cplusplus
extern "C"{
#endif

#define print_fmt		"%s:%s:%s\n"
#define print_fmt_fn		"@%s\n"
#define _strlen(x)		(((x)==NULL)?0:strlen(x))
#define print_fmt_len(a,t,v)	(_strlen((a))+_strlen((t))+_strlen((v))+2+1)

//
// # cat ./aa?json
// # cat ./aa?wait,deleta  
//
#define PATH_OPTION_DELIM	'?'
#define PATH_OPTION_SEP		","

//
// "attribute":json:xxxxx
//
#define MAX_ATTRIB_LEN		32
#define MAX_ATTRIB_TYPE		(MAX_ATTRIB_LEN+1+4+1)

//
// file options..
// 	"?json"
// 	"?xml"
// 	"?wait"
// 	"?delta"
// 	"?deltadir"
// 	"?debug"
// 	"?user"
//
// Higher 16bits - offset value for "?debug"
//
#define OPTION_JSON		(0x1  & 0xFFFF)
#define OPTION_XML		(0x2  & 0xFFFF)
#define OPTION_WAIT		(0x4  & 0xFFFF)
#define OPTION_DELTA		(0x8  & 0xFFFF)
#define OPTION_DELDR		(0x10 & 0xFFFF)
#define OPTION_STOP		(0x20 & 0xFFFF)
#define OPTION_DEBUG		(0x40 & 0xFFFF)
#define OPTION_USER		(0x80 & 0xFFFF)

//
// buffer increment unit 
// temporal buffer size 
//
#define INCUNIT	4

//
//
// base list structure...
typedef struct __list {
        struct __list *next;
        struct __list *prev;
}__attribute__((aligned)) list_t;

//
// History block ...
typedef struct {
	#define LPPS_HIST_BASE 0x100
	#define LPPS_HIST_MKFILE	(LPPS_HIST_BASE+1)
	#define LPPS_HIST_RMFILE	(LPPS_HIST_BASE+2)
	#define LPPS_HIST_MKDIR		(LPPS_HIST_BASE+3)
	#define LPPS_HIST_RMDIR		(LPPS_HIST_BASE+4)
	#define LPPS_HIST_NULL		(LPPS_HIST_BASE+5)
	int mode;
	char *path;
}__attribute__((aligned)) lpps_hist_blk_t;

//
// history queue - 
// Upon numerous testing, the following strucure is fixed,
//   we need lock-free operation..
//
typedef struct {
	int head;  // get
	int tail;  // put 
	int size;
	lpps_hist_blk_t **blks;
}__attribute__((aligned)) lpps_hist_queue_t;

//
//                   Data Structure 
//
//   +----------+------------+------------+--------+
//   |          |            |            |        |
//   |   ppsl   | ppsitem[0] | ppsitem[1] |  ....  |
//   |          |            |            |        |
//   +----------+------------+------------+--------+
//
typedef struct __ppsitem {
	char *attrib;
	char *type;
	char *value;
}__attribute__((aligned)) ppsitem;

typedef struct __ppsl{
	int bytelen;	 // byte length 
	int count; 	 // size of PPS items...
	int occupancy;	 // population 
	//int rptr;        // partial read pointer - previous read count 

	//
	// Simply notation for consecutive addressing..
	//
	ppsitem item[0]; 
}__attribute__((aligned)) ppsl;
#define tuples_size(l)		((l)->occupancy)

// private data structure...
//
// This structure depends on access instances.
// Multiple instances of this structure can be created for one file. - Different & unique file handle (fh) is assigned.  
//
typedef struct __ppsl_priv {
	int fh; 	  // file handle 
	int unique;	  // unique transaction ID 
	os_sig_t *sig;    // signalling 
	#define ACK_RELEASE   0x0800
	#define ACK_SHUTDOWN  0x8000
	os_evt_t *ackevt; // non-overlapped execution between consumer(read) and producer(write)
	int rptr;	  // read pointer 
	short nth;	  // nth location of DB 
	#define READ_WORK 	0
	#define WRITE_WORK 	1
	short mode;	  // READ/WRITE
	int flags;	  // flags - currently OPTION_JSON
	int buflen;	  // size of linebuf (temporal I/O buffer)
	char * linebuf;	  // temporal I/O buffer 

	// history tracking 
	char * path;
	lpps_hist_queue_t hq;
	list_t l;
}__attribute__((aligned)) ppriv;
#define priv_eof(l,priv)		(tuples_size(l) <= (((ppriv *)(priv))->rptr))
#define priv_set(priv,v)		{((ppriv *)(priv))->rptr = (v);}
#define priv_rewind(priv)		priv_set(((ppriv *)priv),0)

extern ppsl * lpps_ppsl_parse( ppsl * list, const char * wbuf, off_t offset, size_t size );
extern int lpps_ppsl_dump( ppsl * node, char * buff, int size, int json, ppriv * priv );
extern int lpps_ppsl_print( ppsl * node );
extern int lpps_ppsl_remove( ppsl ** node );
extern ppsl * lpps_ppsl_merge( ppsl * root , ppsl * node , ppsl **delta );
extern ppsl * lpps_ppsl_duplicate( ppsl * );
extern int lpps_ppsl_concat( ppsl ** node , ppsl * appended );
extern void lpps_ppsl_sort( ppsl * node , int start , int end );

#ifdef __cplusplus
};
#endif

#endif /* __PPS_LIST_HEADER__ */
