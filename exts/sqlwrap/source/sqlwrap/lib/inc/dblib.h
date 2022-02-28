#ifndef _DATABASE_LIBRARY_HEADER_
#define _DATABASE_LIBRARY_HEADER_

#ifdef __cplusplus
extern "C" {
#endif

/* SQL lite header file */
#include "sqlite3.h"
/* Base component header file */
#include "oskit.h"

#ifndef NET_ADDR_LEN
/* address information */
#define NET_ADDR_LEN  32
typedef struct {
	unsigned int ip;        /* ip address */
	unsigned int port;          /* port # */
	unsigned char addr[NET_ADDR_LEN]; /* struct sockaddr_in */
	unsigned int addrsize;      /* address size */
}net_addrinfo_t;
#endif

/* Channel number for DB server */
#define DB_SERVER_CHANNEL_NUMBER  4405
#define DB_CLIENT_CHANNEL_NUMBER  4406
#define DB_MAX_NETWORK_PAYLOAD    (8*1024)

#define DB_NET_OVERHEAD       (64)
#define DB_PACKET_PAYLOAD_SZ      (DB_MAX_NETWORK_PAYLOAD-DB_NET_OVERHEAD)

/* Network information */
typedef struct {
	unsigned int socket_port; /* real TCP socket port*/
	net_addrinfo_t addr; /* address information */
	unsigned int serv_port; /* port */
	unsigned int payload;  /* size of payload */
}db_netinfo_t;

#define FILENAME_LEN 128
#define DATABASE_FILENAME "/tmp/test.dbf"

#define DBSVR_BUFF_CLASS_ID   0x39392341
#define DBSVR_BUFF_CLASS_SIZE (256*1024)

/* Database object structure */
typedef struct {
	unsigned char name[FILENAME_LEN];
	sqlite3 *db;
	/* command buffer class */
	unsigned int cmd_bcls;
	/* DB network information */
	db_netinfo_t netinfo;
#ifdef SQLITE_SINGLE
	unsigned int tid;
#endif
}__attribute__((aligned)) db_t;

/*
 * Database query data structure...
 */
typedef struct {

  #define DB_TYPE_INTEGER 0x00000001
  #define DB_TYPE_STRING  0x00000002
	unsigned int type;
#define DB_ITEM_TYPE(x)    ((x) ? ((x)->type) : 0)

	unsigned int intval;
#define DB_ITEM_INTVAL(x)  ((x) ? ((x)->intval) : 0)

	/* string container */
	unsigned char *strval;
#define DB_ITEM_STRVAL(x)  ((x) ? ((x)->strval) : 0)

	unsigned int lenstr;
#define DB_ITEM_STRLEN(x)  ((x) ? ((x)->lenstr) : 0)

}__attribute__((aligned)) db_result_list_item;           /* Each column info */

typedef struct db_result_list_column_t {

	unsigned int count;
	db_result_list_item *items;

	struct db_result_list_column_t *next;
}__attribute__((aligned)) db_result_list_column;

/* Query result function */
typedef struct {

	int ret;    /* return value */

  #define ERR_MSG_LEN 64
	char *errmsg; /* error message */

	/* table list... */
	unsigned int count;

	db_result_list_column *items;

}__attribute__((aligned)) db_result_list;

/* Network packet header */
typedef struct {

#define DBMAN_CLIENT_CMD_SQL   0x00aac001
#define DBMAN_CLIENT_CMD_STOP  0x00aac002
#define DBMAN_CLIENT_CMD_TERM  0x00aac003
	unsigned int cmd; /* command */

	unsigned int xid; /* SQL transaction ID */

	unsigned int len; /* length of packet */

	unsigned char payload[DB_PACKET_PAYLOAD_SZ];

#ifdef SQLITE_SINGLE
	db_result_list *result;
	unsigned int sync;
#endif

}__attribute__((packed)) db_net_packet_t;

/*
 * error code
 */
extern int dbman_result_error(db_result_list *);

/*
 * error code
 */
extern char * dbman_result_errmsg(db_result_list *);

/*
 * size of list...
 */
extern int dbman_result_list_size(db_result_list *);

/*
 * Returning an indexed column,
 */
extern db_result_list_column * dbman_fetch_column(db_result_list *,int);

/*
 * Returning an indexed item,
 */
extern db_result_list_item * dbman_fetch_item(db_result_list_column *,int);

/*
 * size of column...
 */
extern int dbman_result_column_size(db_result_list_column *);

/*
 * freeing up result list
 */
extern int dbman_result_free(db_result_list *);

/* result callback function */
typedef int (*dbman_result_callback_f)(unsigned int,db_result_list *);

/* Database command structure */
typedef struct {

#define DB_NORES 0x100  /* Fatal error */
	int res;

	/* transaction id */
	unsigned int xid;

	/* command location/size */
	unsigned char *cmd;
	unsigned int len;

	/* result callback function */
	dbman_result_callback_f res_f;

	/* result list */
	db_result_list *result_list;

	/* signalling...*/
	unsigned int sig;
}__attribute__((aligned)) db_cmd_t;

/* should created once/configured once */
extern unsigned int db_buffer_id;
#define db_mem_alloc(sz)    buff_alloc_data(db_buffer_id,(sz))
#define db_mem_free(obj)    buff_free_data(db_buffer_id,(void *)(obj))

/* Database command */
#define DB_CMD_COMMAND   0x00aa00fc
#define DB_CMD_STOP      0x00aa00fd

/*
 * F U N C T I O N S
 *
 */

/************************************************
 *
 * SERVER LIBRARIES...
 *
 ***********************************************/

/*
 * Initialization of Database Service
 *
 */
extern int dbman_service_initialize(unsigned int port,const char *dbfname,unsigned char *logfile);

/*
 * Shutdown of Database Service
 *
 */
extern int dbman_service_shutdown(void);

/************************************************
 *
 * CLIENT LIBRARIES...
 *
 ***********************************************/

#define DBCLI_BUFF_CLASS_ID   0x39392342
#define DBCLI_BUFF_CLASS_SIZE DBSVR_BUFF_CLASS_SIZE

/* Database client structure */
typedef struct {

	net_addrinfo_t addr;

	unsigned int channel; /* channel */

	unsigned int port;  /* port */

	unsigned int thread; /* receiver thread */

}db_client_t;

/*
 * Connecting to a server
 *
 */
extern db_client_t * dbman_connect_service(unsigned int ip,unsigned int port,unsigned char *logfile);

/*
 * Command executor in client...
 *
 * dbman_run_sql_command  - Running SQL command .....
 *
 */
extern db_result_list * dbman_run_sql_command(
	db_client_t *instance,     /* Return values from dbman_connect_service */
	unsigned int xid,         /* transaction ID */
	const char *sqlcmd);      /* SQL command */

/*
 * Command executor in client...
 *
 * dbman_result_traverse - Traversing SQL result list
 *
 */
typedef void (*dbman_traverse_function_t)(const int,const int,const int,void *,const int);
extern int dbman_result_traverse(db_result_list *result,dbman_traverse_function_t func);

/*
 * Disconnecting from a server
 *
 */
extern int dbman_disconnect_service(db_client_t *instance);
/* instance -> returned from dbman_connect_service */

/************************************************
 *
 * ENCTRYPT / DECRYPT
 *
 ***********************************************/
extern int dbman_sql_encrypt_result(unsigned char *space,unsigned int sz,db_result_list *result);
extern db_result_list * dbman_sql_decrypt_result(unsigned char *space,unsigned int sz);

/************************************************
 *
 * DEBUGGING
 *
 ***********************************************/
extern unsigned int dbman_printf_init(unsigned char *file_name);
extern unsigned int dbman_printf_shutdown(void);
#define DBMAN_DBG_LVL_ERR  0x01
#define DBMAN_DBG_LVL_MSG  0x02
extern unsigned int dbman_printf(int lvl,const char *fmt, ...);
extern void dbman_printf_level(int lvl);

#ifdef __cplusplus
}
#endif

#endif
