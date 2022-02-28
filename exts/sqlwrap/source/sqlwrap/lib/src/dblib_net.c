#include <stdio.h>
#include <stdlib.h>
#include "oskit.h"
#include "dblib.h"

/* UTVBono Database Descriptor */
static db_t *db = NULL;
/* UTVBono Database initialization flag */
static unsigned int db_inited = 0;

/* will be assigned in dbman_service_initialize for server,
    and dbman_connect_server for client */
unsigned int db_buffer_id = 0;

static os_mutex_t port_set_lock;

#define DBMAN_MAX_CLIENTS 5
struct _dbman_current_channel_port_set {
	unsigned int portid;
	unsigned int channelid;
	unsigned int exec_continue;
	unsigned int thr;
	unsigned int avail;
} __attribute__((aligned)) dbman_current_channel_port_set[DBMAN_MAX_CLIENTS] = { {0,0,0,0}, };

/**************************************************
 *
 * S T A T I C        F U N C T I O N S
 *
 */

/*
 * Generic result handler...
 *
 */
static int dbman_generic_result_callback(unsigned int xid,db_result_list *res)
{
	if(res) {
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_generic_result_callback(%08x)::result=%d\n",xid,res->ret);
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_generic_result_callback(%08x)::errmsg=%s\n",xid,(res->errmsg) ? res->errmsg : "NULL");
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_generic_result_callback(%08x)::rows=%d\n",xid,res->count);
		dbman_result_free(res);
	}else
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_generic_result_callback(%08x)::null result\n",xid);

	return 1;
}

/*
 * Returning the list of items....
 *
 */
static db_result_list * dbman_execute_produce_result_list(sqlite3_stmt *result)
{
	int ix;
	int res;
	int row;
	db_result_list *l;
	db_result_list_column *l_column,*l_prev_column;
	db_result_list_item   *l_item;

	l = (db_result_list *)db_mem_alloc(sizeof(db_result_list));
	if(!l) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_execute_produce_result_list::db_mem_alloc(..db_result_list)::error\n");
		return NULL;
	}
	/* initializing... */
	l->items  = NULL;
	l->errmsg = NULL;
	l->ret    = 0;
	l->count  = 0;

	/* starting... */
	l_prev_column = NULL;
	row = 0;

	/* scanning... */
	while(1) {
		/* Execute... */
		res=sqlite3_step(result);
		if(res == SQLITE_DONE) {
			/* Done !! */
			break;
		}else
		if(res == SQLITE_ROW) {
			/* Success !! */
			++row;
		}else{
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_execute_produce_result_list::sqlite3_step::(res=%08x)\n",res);
			break;
		}

		/* create a column */
		l_column = (db_result_list_column *)db_mem_alloc(sizeof(db_result_list_column));
		if(!l_column) {
			dbman_result_free(l);
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_execute_produce_result_list::db_mem_alloc(..db_result_list_column)::error\n");
			return NULL;
		}

		l_column->count = sqlite3_column_count(result);         /* column count */
		if(l_column->count <= 0) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_execute_produce_result_list::sqlite3_column_count(result)=%d\n",
			             l_column->count);
			dbman_result_free(l);
			return NULL;
		}
		/* next pointer */
		l_column->next = NULL;

		/* item creation */
		l_column->items = (db_result_list_item *)db_mem_alloc(
			sizeof(db_result_list_item)*l_column->count);
		if(!l_column->items) {
			dbman_result_free(l);
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_execute_produce_result_list::db_mem_alloc(..db_result_list_item)*count::error\n");
			return NULL;
		}

		/* one row... */
		for(ix=0; ix<l_column->count; ix++) {
			/* each column processing */
			l_item = &(l_column->items[ix]);
			switch(sqlite3_column_type(result,ix)) {
			case SQLITE_INTEGER:
				l_item->type = DB_TYPE_INTEGER;
				l_item->intval = sqlite3_column_int(result,ix);
				dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_execute_produce_result_list::integer=%d\n",l_item->intval);
				break;
			case SQLITE_TEXT:
				l_item->type = DB_TYPE_STRING;
				l_item->lenstr = sqlite3_column_bytes(result,ix);
				if(l_item->lenstr>0) {
					l_item->strval = (unsigned char *)db_mem_alloc(l_item->lenstr);
					memcpy( l_item->strval,
					        sqlite3_column_text(result,ix),
					        l_item->lenstr);
				}else
					l_item->strval = NULL;
				dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_execute_produce_result_list::string=%s\n",l_item->strval);
				break;
			default:
				dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_execute_produce_result_list::unrecognizable type\n");
				break;
			}
		}
		if(l_prev_column == NULL) {
			/* first column */
			l->items = l_column;
		}else{
			l_prev_column->next = l_column;
		}
		l_prev_column = l_column;
	}     /* end of while(...) */

	/* checking return value */
	l->ret = (res == SQLITE_DONE) ? SQLITE_OK : res;
	if(l->ret != SQLITE_OK) {
		/* abnormal return value */
		l->errmsg = (char *)db_mem_alloc(ERR_MSG_LEN);
		if(!l->errmsg) {
			dbman_result_free(l);
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_execute_produce_result_list::db_mem_alloc(..errmsg)::error\n");
			return NULL;
		}
		strncpy(l->errmsg,sqlite3_errmsg(db->db),ERR_MSG_LEN);
	}else
		l->errmsg = NULL;

	l->count = row;
	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_execute_produce_result_list::done with %d rows\n",row);

	return l;
}

/*
 * Database processor
 *
 */
static int dbman_proc(int type,db_cmd_t *cmd)
{
	int res;
	int ix;
	sqlite3_stmt *resultStmt;     /* result buffer */
	db_result_list *reformed_result;

	if(!cmd) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_proc::null command::system broken\n");
		return -1;
	}

	/* command processing */
	switch(type) {
	case DB_CMD_COMMAND:

		#ifdef SQLITE_LAZY_PROC
		/* Database opening ... */
		res = sqlite3_open(db->name,&(db->db));
		if(res!=SQLITE_OK) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_service_initialize::database::not opened\n");
			break;
		}
		dbman_printf(DBMAN_DBG_LVL_MSG,"DB(%s) opening success\n",db->name);
		#endif

		/* table processing... */
		res = sqlite3_prepare_v2(db->db,
		                         cmd->cmd,
		                         strlen(cmd->cmd),
		                         &resultStmt,         /* statement - result */
		                         0);
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_proc::normal command(%s)=%s\n",
		             cmd->cmd,
		             (res!=SQLITE_OK) ? "failed" : "succeeded");

		/* collecting result... */
		reformed_result =
			dbman_execute_produce_result_list(resultStmt);
		if(!reformed_result) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"Fatal system error!!....produce_result_list::NULL\n");
			exit(0);
		}

		/* SQL finalizing */
		if(reformed_result->ret==
		   SQLITE_OK) {
			if(sqlite3_finalize(resultStmt)!=SQLITE_OK) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_proc::finalize error\n");
			}
		}

		#ifdef SQLITE_LAZY_PROC
		/* Database opening ... */
		res = sqlite3_close(db->db);
		if(res!=SQLITE_OK) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_service_initialize::database::not closed\n");
			break;
		}
		dbman_printf(DBMAN_DBG_LVL_MSG,"DB(%s) close success\n",db->name);
		#endif

		/* return value */
		cmd->res = (reformed_result) ? reformed_result->ret : DB_NORES;

		/* result list */
		cmd->result_list = reformed_result;

		/* result callback function call... */
		if(cmd->res_f) cmd->res_f(cmd->xid,reformed_result);

		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_proc::execution::done(result_list=%08x)\n",
		             (unsigned long)reformed_result);

		break;

	case DB_CMD_STOP:
		/* signalling - wakeup */
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_proc::stop command\n");
		return 0;
	default:
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_proc::unknown command::broken\n");
		break;
	}
	return 0;
}

/*
 * Queue callback function
 *
 */
static void dbman_q_callback(void *obj)
{
	unsigned long maddr = *(unsigned long *)obj;
	OS_MESG *msg;
	db_cmd_t *cmd;

	/* message block */
	msg = (OS_MESG *)maddr;
	if(!msg) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_q_callback::null message::system broken!!\n");
		return;
	}

	/* command block */
	cmd = (db_cmd_t *)OS_MESG_DATA(msg);
	if(!cmd) {
		db_mem_free(msg);
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_q_callback::null db command::system broken!!\n");
		return;
	}

	/* sig deletion */
	os_cond_delete(cmd->sig);
	/* command string deallocation */
	db_mem_free(cmd->cmd);
	/* command deallocation */
	db_mem_free(cmd);
	/* message deallocation */
	db_mem_free(msg);

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_q_callback::done\n");
	return;
}

/*
 * dbman_current_channel_port_set - insert information
 */
static int dbman_new_comm_channel_insert(unsigned int port,unsigned int channel)
{
	int ii;

	for(ii=0; ii<DBMAN_MAX_CLIENTS; ii++) {
		if(dbman_current_channel_port_set[ii].avail == 0) {
			dbman_current_channel_port_set[ii].portid = port;
			dbman_current_channel_port_set[ii].channelid = channel;
			dbman_current_channel_port_set[ii].avail = 1;
			dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_new_comm_channel_insert::at %d\n",ii);
			return 1;
		}
	}
	dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_new_comm_channel_insert::full!!\n");
	return 0;
}

#if 0
/*
 * dbman_current_channel_port_set - activate information
 */
static int dbman_new_comm_channel_activate(unsigned int port,unsigned int thr)
{
	int ii;

	for(ii=0; ii<DBMAN_MAX_CLIENTS; ii++) {
		if(dbman_current_channel_port_set[ii].avail == 1) {
			if( dbman_current_channel_port_set[ii].portid == port ) {
				dbman_current_channel_port_set[ii].thr = thr;
				dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_new_comm_channel_activate::at %d\n",ii);
				return 1;
			}
		}
	}
	dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_new_comm_channel_activate::full!!\n");
	return 0;
}
#endif

/*
 * dbman_current_channel_port_set - search information
 */
static unsigned int dbman_new_comm_channel_search(unsigned int port)
{
	int ii;
	unsigned int channel;

	for(ii=0; ii<DBMAN_MAX_CLIENTS; ii++) {
		if(dbman_current_channel_port_set[ii].avail == 1) {
			if( dbman_current_channel_port_set[ii].portid == port ) {
				channel = dbman_current_channel_port_set[ii].channelid;
				dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_new_comm_channel_search::at %d\n",ii);
				return ii+1;
			}
		}
	}
	dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_new_comm_channel_search::failed!!\n");
	return 0;
}

/*
 * dbman_current_channel_port_set - delete information
 */
static int dbman_new_comm_channel_delete(unsigned int port)
{
	int ii;
	unsigned int channelnum;

	for(ii=0; ii<DBMAN_MAX_CLIENTS; ii++) {
		if(dbman_current_channel_port_set[ii].avail == 1) {
			if( dbman_current_channel_port_set[ii].portid == port ) {
				//os_thread_delete( dbman_current_channel_port_set[ii].thr );
				if( net_channel_flush( dbman_current_channel_port_set[ii].channelid ) == ERROR ) {
					dbman_printf(DBMAN_DBG_LVL_ERR,
					             "..comm_channel_delete::channel_flush::error(%08x)\n",dbman_current_channel_port_set[ii].channelid);
				}
				channelnum = dbman_current_channel_port_set[ii].channelid;
#if 0
				if( net_channel_delete( dbman_current_channel_port_set[ii].channelid ) == ERROR ) {
					dbman_printf(DBMAN_DBG_LVL_ERR,
					             "..comm_channel_delete::channel_delete::error(%08x)\n",dbman_current_channel_port_set[ii].channelid);
				}
				if( net_port_delete( dbman_current_channel_port_set[ii].portid ) == ERROR ) {
					dbman_printf(DBMAN_DBG_LVL_ERR,
					             "..comm_channel_delete::port_delete::error(%08x)\n",dbman_current_channel_port_set[ii].portid);
				}
#endif
				dbman_current_channel_port_set[ii].avail = 0;
				dbman_printf(DBMAN_DBG_LVL_MSG,"..comm_channel_delete::deleted channel = %08x\n",channelnum);
				return 1;
			}
		}
	}
	dbman_printf(DBMAN_DBG_LVL_ERR,"..comm_channel_delete::failed!!\n");
	return 0;
}

/*
 * Command executor
 *
 */
static int dbman_callback_execute(unsigned int transid,
                                  const char *sqlcmd,dbman_result_callback_f result_f)
{
	int res;
	int len = strlen(sqlcmd);
	db_cmd_t *cmd;

	if(!db_inited) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_callback_execute::not yet initialized\n");
		return -1;
	}

	if(!len) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_callback_execute::null command\n");
		return -1;
	}

	/* command block allocation */
	cmd = (db_cmd_t *)db_mem_alloc(sizeof(db_cmd_t));
	if(!cmd) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_callback_execute::allocation failed\n");
		return -1;
	}

	cmd->cmd = (unsigned char *)db_mem_alloc(len+32);
	if(!cmd->cmd) {
		db_mem_free(cmd);
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_callback_execute::command block::allocation failed\n");
		return -1;
	}
	memset(cmd->cmd,0,len+32);     /* zeroing buffer */

	/* filling up structure */
	cmd->xid = transid;     /* transaction id */
	memcpy(cmd->cmd,sqlcmd,len);
	cmd->len = len;
	cmd->res_f = (!result_f) ? dbman_generic_result_callback : result_f;
	/* result callback function, if NULL -> generic handler attached */

	if( dbman_proc(DB_CMD_COMMAND,cmd) ) {
		db_mem_free(cmd->cmd);
		db_mem_free(cmd);
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_callback_execute::message send::failed\n");
		return -1;
	}

	res = cmd->res;     /* return values */

	/* deleting resources */
	db_mem_free(cmd->cmd);
	db_mem_free(cmd);

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_callback_execute::done\n");
	return res;
}

/*
 * Command executor
 *
 */
static db_result_list * dbman_sql_execute(const char *sqlcmd)
{
	int res;
	int len = strlen(sqlcmd);
	db_cmd_t *cmd;
	db_result_list *result_l;

	if(!db_inited) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_execute::not yet initialized\n");
		return NULL;
	}

	if(!len) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_execute::null command\n");
		return NULL;
	}

	/* command block allocation */
	cmd = (db_cmd_t *)db_mem_alloc(sizeof(db_cmd_t));
	if(!cmd) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_execute::allocation failed\n");
		return NULL;
	}

	cmd->cmd = (unsigned char *)db_mem_alloc(len+32);
	if(!cmd->cmd) {
		db_mem_free(cmd);
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_execute::command block::allocation failed\n");
		return NULL;
	}
	memset(cmd->cmd,0,len+32);     /* zeroing buffer */

	/* filling up structure */
	cmd->xid = 0;     /* NULL transaction id */
	memcpy(cmd->cmd,sqlcmd,len);
	cmd->len = len;
	cmd->res_f = NULL;     /* result function is NULL */

	if( dbman_proc(DB_CMD_COMMAND,cmd) ) {
		db_mem_free(cmd->cmd);
		db_mem_free(cmd);
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_execute::message send::failed\n");
		return NULL;
	}

	result_l = cmd->result_list;

	/* deleting resources */
	//os_cond_delete(cmd->sig);
	db_mem_free(cmd->cmd);
	db_mem_free(cmd);
	//db_mem_free(msg);

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_execute::done\n");
	return result_l;
}

/*
 *
 * Server for individual client
 *
 */
static os_thrfunc_t dbman_client_server(void *arg)
{
	int res;
	unsigned int sport;
	unsigned int dport;
	unsigned int channel,port;
	db_net_packet_t *db_client_cmd;
	net_packet *packet;
	db_result_list *db_result;
	struct _dbman_current_channel_port_set *channel_port_set;

	if(!arg) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_client_server::null argument\n");
		return;
	}

	channel_port_set = (struct _dbman_current_channel_port_set *)arg;     /* current channel */
	channel = channel_port_set->channelid;
	port    = channel_port_set->portid;
	dbman_printf(DBMAN_DBG_LVL_MSG,"..dbman_client_server::channel=%08x,port=%08x\n",channel,port);

	while(channel_port_set->exec_continue) {
		packet = net_packet_recv(channel,0);
		if(!packet) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"..dbman_client_server::net_packet_recv::NULL packet\n");
			dbman_printf(DBMAN_DBG_LVL_ERR,"..dbman_client_server::net_packet_recv::Deleting...\n");
			/* port delete ... */
			channel_port_set->exec_continue = 0;
			continue;
		}
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_client_server::net_packet_recv:okay\n");

		/* command block fetch... */
		db_client_cmd = (db_net_packet_t *)NET_PACKET_PAYLOAD(packet);
		if(!db_client_cmd) {
			net_packet_free(channel,packet);
			dbman_printf(DBMAN_DBG_LVL_ERR,"..dbman_client_server::net_packet_recv::NULL payload\n");
			continue;
		}
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_client_server::db_client_cmd=%x\n",ntohl(db_client_cmd->cmd));

		/* Command type ... */
		switch(ntohl(db_client_cmd->cmd)) {
		case DBMAN_CLIENT_CMD_SQL:

			/* SQL processing.... */
			db_result = dbman_sql_execute(db_client_cmd->payload);
			dbman_printf(DBMAN_DBG_LVL_MSG,"DBMAN_CLIENT_CMD_SQL::RES::(%s)::%s\n",
			             db_client_cmd->payload,(db_result) ? "Success" : "Failure");

			/* sending result list */
			if(dbman_sql_encrypt_result(
				   db_client_cmd->payload,
				   DB_PACKET_PAYLOAD_SZ,
				   db_result)) {
				if(db_result) dbman_result_free(db_result);
				dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_response()::error\n");
				break;
			}

			/* clean up result list */
			if(db_result) dbman_result_free(db_result);

			dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_result_free(db_result)::done\n");

			/* Deleting a packet */
			if( net_packet_free(channel,packet) == ERROR ) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"net_packet_free::error\n");
				break;
			}

			/* Building a new packet */
			packet = net_packet_make(
				channel,
				DB_CLIENT_CHANNEL_NUMBER,
				0,                 /* message type */
				sizeof(db_net_packet_t),
				0,                 /* option */
				db_client_cmd->payload);
			if(!packet) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::net_packet_make::response::error\n");
				break;
			}

			/* sending reply packet */
			if(net_packet_send(channel,packet) == ERROR) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"DBMAN_CLIENT_CMD_SQL::net_packet_send::error\n");
			}else{
				dbman_printf(DBMAN_DBG_LVL_MSG,"DBMAN_CLIENT_CMD_SQL::done\n");
			}

			/* freeing up packet TXed */
			if(db_mem_free(packet)==ERROR) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"db_mem_free::TXed\n");
			}

			break;             /* .. _CMD_SQL */
		case DBMAN_CLIENT_CMD_STOP:
			/* STOP Service */
			dbman_printf(DBMAN_DBG_LVL_MSG,"DBMAN_CLIENT_CMD_STOP::RES::(%08x)\n",channel);
			/* Deleting a packet */
			if(net_packet_free(channel,packet) == ERROR) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"net_packet_free::RXed::error\n");
			}
			strcpy(db_client_cmd->payload,"OK");
			/* Building a new packet */
			packet = net_packet_make(
				channel,
				DB_CLIENT_CHANNEL_NUMBER,
				0,                 /* message type */
				sizeof(db_net_packet_t),
				0,                 /* option */
				db_client_cmd->payload);
			if(!packet) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::net_packet_make::response::error\n");
				break;
			}
			/* sending reply packet */
			if(net_packet_send(channel,packet) == ERROR) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"DBMAN_CLIENT_CMD_STOP::net_packet_send::error\n");
			}
			/* freeing up packet TXed */
			if(db_mem_free(packet)==ERROR) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"db_mem_free::TXed\n");
			}
			channel_port_set->exec_continue = 0;
			dbman_printf(DBMAN_DBG_LVL_MSG,"DBMAN_CLIENT_CMD_STOP::done\n");
			break;             /* _CMD_STOP */
		default:
			/* Unknown command */
			if(net_packet_free(channel,packet) == ERROR) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"net_packet_free::RXed\n");
			}
			dbman_printf(DBMAN_DBG_LVL_MSG,"DBMAN_CLIENT_CMD_xxxxx=UNKNOWN\n");
			break;
		}
		/* freeing up packet received */
	}

  #if 0
	/* deleting channel-port information */
	if( !dbman_new_comm_channel_delete(port) ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::...comm_channel_delete::error\n");
		//return -1
	}

	/* Deleting a port */
	dbman_printf(DBMAN_DBG_LVL_ERR,"client_server::port_delete(%08x)::trying..\n",port);
	if( net_port_delete(port) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"client_server::port_delete(%08x)::failed\n",port);
	}
  #endif

	/* Reource cleanup */
	if( net_port_cleanup(port) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"client_server::port_cleanup(%08x)::failed\n",port);
	}

	/* ending message */
	dbman_printf(DBMAN_DBG_LVL_MSG,"..dbman_client_server::stopped(channel=%08x)\n",channel);
}

/*
 * upcall function for DB multi-server
 */
static void dbman_server_port_callback(unsigned int id,unsigned int command,void *arg)
{
	unsigned int serv_channelid;
	unsigned int newportid;
	unsigned int index;
	struct _dbman_current_channel_port_set *channel_port_set;

	switch(command) {
	case NET_MESG_ACCEPT:
		dbman_printf(DBMAN_DBG_LVL_MSG,"..server_port_callback::MESG_ACCEPT\n");

		newportid = *(unsigned int *)arg;         /* new port */
		dbman_printf(DBMAN_DBG_LVL_MSG,"upcall=new port id(%08x)\n",newportid);

		/* server */
		serv_channelid = net_channel_create(
			DBSVR_BUFF_CLASS_ID,             /* buffer class */
			DB_SERVER_CHANNEL_NUMBER             /* channel number */
			);
		if(serv_channelid == ERROR) {
			dbman_printf(DBMAN_DBG_LVL_ERR,
			             "..server_port_callback::error in net_channel_create(%d)\n",
			             DB_SERVER_CHANNEL_NUMBER);
			break;
		}

		/* registering */
		if(net_channel_register( newportid,
		                         serv_channelid)==ERROR) {
			net_channel_delete(serv_channelid);
			dbman_printf(DBMAN_DBG_LVL_ERR,
			             "..server_port_callback::error in net_channel_port_set()::error\n");
			break;
		}

		_os_mutex_lock( &port_set_lock );

		dbman_printf(DBMAN_DBG_LVL_MSG,"..server_port_callback::new channel is %08x\n",serv_channelid);
		if( !dbman_new_comm_channel_insert( newportid, serv_channelid ) ) {
			net_channel_delete(serv_channelid);
			dbman_printf(DBMAN_DBG_LVL_ERR,
			             "..server_port_callback::channel(%08x)/port(%08x) error\n", serv_channelid, newportid );
		}

		_os_mutex_unlock( &port_set_lock );
		break;

	case NET_MESG_NEW_PORT:
		dbman_printf(DBMAN_DBG_LVL_MSG,"..server_port_callback::NEW_PORT\n");

		newportid = *(unsigned int *)arg;         /* new port */

		_os_mutex_lock( &port_set_lock );

		/* channel number searching... */
		index = dbman_new_comm_channel_search(newportid);
		if(!index) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"..server_port_callback::NEW_PORT::failed to find out channel for port=%08x\n",newportid);
			_os_mutex_unlock( &port_set_lock );
			break;
		}
		--index;
		channel_port_set = &(dbman_current_channel_port_set[index]);
		serv_channelid = channel_port_set->channelid;

		dbman_printf(DBMAN_DBG_LVL_MSG,"..server_port_callback::channelid=%08x\n",serv_channelid);

#if 0
		/* channel activation */
		if( !dbman_new_comm_channel_activate( newportid, channel_port_set->thr ) ) {
			net_channel_delete( serv_channelid );
			dbman_new_comm_channel_delete( newportid );
			dbman_printf(DBMAN_DBG_LVL_ERR,"port(%08x)/thread (%08x) activation-failed\n",
			             newportid,new_client_thr);
			_os_mutex_unlock( &port_set_lock );
			break;
		}
#endif
		channel_port_set->exec_continue = 1; /* flags */

		/* creating a thread */
		channel_port_set->thr = os_thread_create("DBchanSrvc",
		                                  (os_thrfunc_t)dbman_client_server,
		                                  (void *)channel_port_set, NULL,0);
		if(channel_port_set->thr == ERROR) {
			net_channel_delete( serv_channelid);
			net_port_delete( newportid );
			dbman_new_comm_channel_delete( newportid );
			dbman_printf(DBMAN_DBG_LVL_ERR,"..server_port_callback::port(%08x) activation-failed\n",newportid);
			_os_mutex_unlock( &port_set_lock );
			break;
		}

		dbman_printf(DBMAN_DBG_LVL_MSG,"..server_port_callback::NEW_PORT - done\n");

		_os_mutex_unlock( &port_set_lock );
		break;

	case NET_MESG_DEL_PORT:
		dbman_printf(DBMAN_DBG_LVL_MSG,"..server_port_callback::DEL_PORT\n");

		newportid = id;

		_os_mutex_lock( &port_set_lock );

		/* channel number searching... */
		index = dbman_new_comm_channel_search(newportid);
		if(!index) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"..server_port_callback::NEW_PORT::failed to find out channel for port=%08x\n",newportid);
			_os_mutex_unlock( &port_set_lock );
			break;
		}
		--index;
		channel_port_set = &(dbman_current_channel_port_set[index]);

		channel_port_set->exec_continue = 0; /* flags */

		dbman_printf(DBMAN_DBG_LVL_MSG,"..Deleing channel will incur thread exit !!\n");

		/* Broadcasting NULL packet to a channel */
		net_port_bcast_channel(net_port_search(id));
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_service_shutdown::broacasting NULL packet to a port(%08x)\n",newportid);

		/* os_thread_delete( channel_port_set->thr ); */
		net_channel_delete( channel_port_set->channelid );
		/* net_port_delete( channel_port_set->portid ); */

		/* deleting channel-port information */
		if( !dbman_new_comm_channel_delete( newportid ) )
			dbman_printf(DBMAN_DBG_LVL_ERR,"serv_port_callback::DEL_PORT::channel delete(id=%08x)::error\n",newportid);

		_os_mutex_unlock( &port_set_lock );

		dbman_printf(DBMAN_DBG_LVL_MSG,"..server_port_callback::DEL_PORT - done\n");
		break;
	}
}

/**************************************************
 *
 * G L O B A L       F U N C T I O N S
 *
 */

/*
 * Shutdown of Database
 *
 */
int dbman_service_shutdown(void)
{
	int res;
	db_cmd_t *cmd;

	if(!db_inited) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_service_shutdown::not inited\n");
		return (-1);
	}

	/*
	 * Killing a thread
	 */
	if( dbman_proc(DB_CMD_STOP,NULL) ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_execute::message send::failed\n");
		return -1;
	}

	/* Deleting a service port */
	if(net_port_delete(db->netinfo.serv_port) == ERROR) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_service_shutdown::net_port_delete::error\n");
		/* return (-1); */         // just go through....
	}

#ifndef SQLITE_LAZY_PROC
	/* Database closing... */
	res = sqlite3_close(db->db);
	if(res) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_service_shutdown::database::not closed\n");
		return (-1);
	}
#endif

	/* Freeing up buffer class */
	if(buff_delete_class(DBSVR_BUFF_CLASS_ID) == ERROR) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_service_shutdown::buff_delete_class:error\n");
		return (-1);
	}
	db_buffer_id = 0;

	dbman_printf_shutdown();

	/* freeing database structure */
	free(db);

	/* not inited !! */
	db_inited = 0;

	printf("dbman_service_shutdown::done\n");
	return 0;
}

/*
 * Initialization of Database
 *
 */
int dbman_service_initialize(unsigned int netport, /* Network port */
                             const char *dbfname,
                             unsigned char *logfile)
{
	int res;

	if(db_inited) {
		printf("dbman_service_initialize::already done\n");
		return (-1);
	}

	dbman_printf_init(logfile);

	db = (db_t *)malloc(sizeof(db_t));
	if(!db) {
		dbman_printf_shutdown();
		printf("dbman_service_initialize::malloc(sizeof(db_t)::error\n");
		return (-1);
	}

	_os_mutex_init( &port_set_lock );

	/* Database file name assignment */
	strcpy(db->name,(dbfname) ? dbfname : DATABASE_FILENAME);

	/* Buffer class for DB processing */
	db->cmd_bcls = buff_create_class(
		DBSVR_BUFF_CLASS_ID,
		NULL,
		DBSVR_BUFF_CLASS_SIZE);
	if(db->cmd_bcls == ERROR) {
		dbman_printf_shutdown();
		free(db);
		printf("dbman_service_initialize::buff_create_class::failed\n");
		return (-1);
	}

	/* buffer id of this system */
	if(db_buffer_id) {
		dbman_printf_shutdown();
		free(db);
		printf("dbman_service_initialize::buffer id has been configured!!\n");
		return (-1);
	}
	db_buffer_id = DBSVR_BUFF_CLASS_ID;

#ifndef SQLITE_LAZY_PROC
	/* Database opening ... */
	res = sqlite3_open(db->name,&(db->db));
	if(res!=SQLITE_OK) {
		dbman_printf_shutdown();
		free(db);
		buff_delete_class(DBSVR_BUFF_CLASS_ID);
		printf("dbman_service_initialize::database::not opened\n");
		return (-1);
	}
#endif

	/* Networking instance created... */
	net_addrinfo_make(&db->netinfo.addr,NET_ANY_IP,netport);

	/* TCP socket port */
	db->netinfo.socket_port = netport;

	/* Creating a service port */
	db->netinfo.serv_port =  net_port_create(
		NET_PORT_TYPE_SERVER,
		NET_PORT_PROTO_TCP,
		&(db->netinfo.addr),
		NULL,
		&dbman_server_port_callback);
	if(db->netinfo.serv_port == ERROR) {
		dbman_printf_shutdown();
		free(db);
		buff_delete_class(DBSVR_BUFF_CLASS_ID);
		printf("dbman_service_initialize::os_thread_create::failed\n");
		return (-1);
	}

	/* increasing payload size */
	db->netinfo.payload = DB_MAX_NETWORK_PAYLOAD;
	if( net_port_option_configure( db->netinfo.serv_port,
	                               NET_PORT_SET_PAYLOAD_SIZE,
	                               &db->netinfo.payload ) == ERROR ) {
		dbman_printf_shutdown();
		net_port_delete(db->netinfo.serv_port);
		free(db);
		buff_delete_class(DBSVR_BUFF_CLASS_ID);
		printf("dbman_service_initialize::port_option_configure...::failed\n");
		return (-1);
	}

	/* Activating a port */
	if(net_port_activate(db->netinfo.serv_port) == ERROR) {
		dbman_printf_shutdown();
		net_port_delete(db->netinfo.serv_port);
		free(db);
		buff_delete_class(DBSVR_BUFF_CLASS_ID);
		printf("dbman_service_initialize::error in net_port_activate()::error\n");
	}

	/* flagging up db inited */
	db_inited = 1;

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_service_initialize::done\n");
	return 0;
}

/*
 * Returning error code
 *
 */
int dbman_result_error(db_result_list *rlist)
{
	return (rlist) ? rlist->ret : 0;
}

/*
 * Returning error message
 *
 */
char * dbman_result_errmsg(db_result_list *rlist)
{
	return (rlist) ? rlist->errmsg : "NULL";
}

/*
 * Returning list size
 *
 */
int dbman_result_list_size(db_result_list *rlist)
{
	return (rlist) ? rlist->count : 0;
}

/*
 * Returning an indexed column,
 *
 */
db_result_list_column * dbman_fetch_column(db_result_list *rlist,int index)
{
	int ix;
	db_result_list_column *col;

	if(!rlist) return NULL;

	if(index > rlist->count) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_fetch_column::out of index(current=%d)\n",rlist->count);
		return NULL;
	}

	col = rlist->items;
	for(ix=0; ix<index; ix++) {
		col = col->next;
		if(col == NULL) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_fetch_column::broken link!!\n");
			return NULL;
		}
	}

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_fetch_column::done\n");
	return col;
}

/*
 * Returning an item in a row,
 *
 */
db_result_list_item * dbman_fetch_item(db_result_list_column *rcol,int index)
{
	int ix;

	if(!rcol) return NULL;

	if(index > rcol->count) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_fetch_item::out of index(current=%d)\n",rcol->count);
		return NULL;
	}

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_fetch_item::done\n");
	return &(rcol->items[index]);
}

/*
 * Size of column...
 *
 */
int dbman_result_column_size(db_result_list_column *rcol)
{
	return (rcol) ? rcol->count : 0;
}

/*
 * Freeing up result list
 *
 */
int dbman_result_free(db_result_list *rlist)
{
	int ix;
	db_result_list_column *rcol,*rcol_tobe_deleted;
	db_result_list_item   *ritem;

	if(!rlist) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_result_free::free list\n");
		return -1;
	}

	if(rlist->errmsg) db_mem_free(rlist->errmsg);     /* error message */

	rcol = rlist->items;
	while(rcol) {
		if(rcol->count) {
			for(ix=0; ix<rcol->count; ix++) {
				ritem = &(rcol->items[ix]);
				if(!ritem) {
					dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_result_free::null item(%d)\n",ix);
					break;
				}
				if(DB_ITEM_TYPE(ritem)==DB_TYPE_STRING) {
					dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_result_free::deleting item(%d)\n",ix);
					db_mem_free(ritem->strval);                     /* string value cleanup */
				}
			}
			dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_result_free::deleting item itself\n");
			db_mem_free(rcol->items);             /* freeing up items */
		}
		rcol_tobe_deleted = rcol;
		rcol = rcol->next;
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_result_free::deleting column\n");
		db_mem_free(rcol_tobe_deleted);
	}
	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_result_free::deleting list itself\n");
	db_mem_free(rlist);     /* deleting rlist itself */

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_result_free::done\n");
	return 0;
}

/*
 * Connecting to a server
 *
 */
db_client_t * dbman_connect_service(
	unsigned int ip,unsigned int port,unsigned char *logfile)
{
	unsigned int max_payload_sz;
	db_client_t *new_client;

	/* channel buffer */
	if( buff_create_class(DBCLI_BUFF_CLASS_ID,NULL,
	                      DBCLI_BUFF_CLASS_SIZE) == ERROR ) {
		printf("dbman_connect_service::buff_create_class::error\n");
		printf("dbman_connect_service::Server/Client should reside in different address spaces.\n");
		return NULL;
	}

	/* buffer id of this system */
	if( db_buffer_id ) {
		printf("dbman_connect_service::buffer_id configured (%x) !!\n", db_buffer_id );
		//return NULL;
	}
	db_buffer_id = DBCLI_BUFF_CLASS_ID;

	/* Lock variable */
	dbman_printf_init(logfile);

	/* Crearting client object */
	new_client = (db_client_t *)db_mem_alloc(sizeof(db_client_t));
	if(!new_client) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"db_mem_alloc::error\n");
		buff_delete_class(DBCLI_BUFF_CLASS_ID);
		dbman_printf_shutdown();
		return NULL;
	}

	/* address create */
	net_addrinfo_make(&(new_client->addr),ip,port);

	/* channel create */
	new_client->channel = net_channel_create(
		DBCLI_BUFF_CLASS_ID,
		DB_CLIENT_CHANNEL_NUMBER         /* channel number */
		);
	if(new_client->channel == ERROR) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"net_channel_create::error\n");
		db_mem_free(new_client);
		buff_delete_class(DBCLI_BUFF_CLASS_ID);
		dbman_printf_shutdown();
		return NULL;
	}

	/* client network port create */
	new_client->port = net_port_create(
		NET_PORT_TYPE_CLIENT,
		NET_PORT_PROTO_TCP,
		&(new_client->addr),
		NULL,
		NULL);
	if(new_client->port == ERROR) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"error in net_port_create()\n");
		net_channel_delete(new_client->channel);
		db_mem_free(new_client);
		buff_delete_class(DBCLI_BUFF_CLASS_ID);
		dbman_printf_shutdown();
		return NULL;
	}

	/* increasing payload size */
	max_payload_sz = DB_MAX_NETWORK_PAYLOAD;
	if( net_port_option_configure( new_client->port,
	                               NET_PORT_SET_PAYLOAD_SIZE,
	                               &max_payload_sz ) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_connect_service::port_option_configure...::failed\n");
		net_channel_delete(new_client->channel);
		db_mem_free(new_client);
		buff_delete_class(DBCLI_BUFF_CLASS_ID);
		dbman_printf_shutdown();
		return (NULL);
	}

	/* socket */
	if( net_port_proto_socket(new_client->port) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"error in net_port_proto_socket()\n");
		net_channel_delete(new_client->channel);
		db_mem_free(new_client);
		buff_delete_class(DBCLI_BUFF_CLASS_ID);
		dbman_printf_shutdown();
		return NULL;
	}

	/* connect */
	if( net_port_proto_connect(new_client->port,NULL) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"error in net_port_proto_connect()\n");
		net_channel_delete(new_client->channel);
		db_mem_free(new_client);
		buff_delete_class(DBCLI_BUFF_CLASS_ID);
		dbman_printf_shutdown();
		return NULL;
	}

	/* Registering channel */
	net_channel_register(new_client->port,new_client->channel);

	/* packet handler start */
	if(net_port_proto_packet_handler_start(new_client->port) == ERROR) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"error in net_port_proto_packet_handler()\n");
		net_port_delete(new_client->port);
		net_channel_delete(new_client->channel);
		db_mem_free(new_client);
		buff_delete_class(DBCLI_BUFF_CLASS_ID);
		dbman_printf_shutdown();
		return NULL;
	}

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_connect_service::done\n");
	return new_client;
}

/*
 * Disconnecting from a server
 *
 */
int dbman_disconnect_service(db_client_t *client)
{
	db_net_packet_t *packet;
	net_packet *term_packet;

	if(!client) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::null client\n");
		return -1;
	}

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_disconnect_service::\n");

	/* packet creation */
	packet = (db_net_packet_t *)db_mem_alloc(sizeof(db_net_packet_t));
	if(!packet) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::db_mem_alloc(db_net_packet_t)::error\n");
		return -1;
	}

	/* filling up header */
	packet->cmd = htonl(DBMAN_CLIENT_CMD_STOP);
	packet->xid = htonl(0);
	packet->len = htonl(0);
	memset(packet->payload,0x00,DB_PACKET_PAYLOAD_SZ);     /* zeroing... */

	/* packet creation */
	term_packet = net_packet_make(
		client->channel,
		DB_SERVER_CHANNEL_NUMBER,
		0,         /* message type */
		sizeof(db_net_packet_t),
		0,         /* option */
		(char *)packet);
	if(!term_packet) {
		db_mem_free(packet);
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::net_packet_make::error\n");
		return (-1);
	}

	/* transmitting a packet */
	if(net_packet_send(client->channel,term_packet) == ERROR) {
		net_packet_free(client->channel,term_packet);
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::net_packet_send::error\n");
		return (-1);
	}

	/* deleting a packet transmitted */
	if(net_packet_free(client->channel,term_packet) == ERROR) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::net_packet_free::error\n");
		return (-1);
	}

	term_packet = NULL;

	/* catching up a reply */
	term_packet = net_packet_recv(client->channel,0);
	if(!term_packet) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::net_packet_recv::error\n");
		//return (-1);
	}

	/* checking payload */
	if(!NET_PACKET_PAYLOAD(term_packet)) {
		db_mem_free(term_packet);
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::net_packet_recv::NULL payload\n");
		//return -1;
	}

	/* Deleting a packet RXed */
	if(net_packet_free(client->channel,term_packet) == ERROR) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::net_packet_free::recv error\n");
		//return -1;
	}

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_disconnect_service::net_channel_flush\n");
	if( net_channel_flush(client->channel) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::net_channel_flush::error\n");
		//return -1;
	}

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_disconnect_service::net_channel_delete\n");
	if( net_channel_delete(client->channel) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::net_channel_delete::error\n");
		//return -1;
	}

	/* deleting port */
	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_disconnect_service::net_port_delete\n");
	if( net_port_delete( client->port ) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::...port_delete::error\n");
		//return -1;
	}

	/* cleaning up port - XXX */
	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_disconnect_service::net_port_cleanup\n");
	if( net_port_cleanup( client->port ) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::...port_cleanup::error\n");
		//return -1;
	}

	/* client structure free */
	if( db_mem_free(client) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::...db_mem_free::error\n");
		//return -1;
	}

	/* deleting buffer class */
	if( buff_delete_class(DBCLI_BUFF_CLASS_ID) == ERROR ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_disconnect_service::...delete_class::error\n");
		//return -1;
	}
	db_buffer_id = 0;

	/* deleting mutex */
	dbman_printf_shutdown();

	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_disconnect_service::done\n");
	return 0;
}

/*
 * Disconnecting from a server
 *
 */
db_result_list * dbman_run_sql_command(db_client_t *client,
                                       unsigned int xid,
                                       const char *sqlcmd )
{
	db_net_packet_t *packet;
	db_result_list *result;
	net_packet *network_packet;
	int len;

	if(!client || !sqlcmd) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"Client/Sqlcommand -> NULL\n");
		return NULL;
	}

	/* length check */
	len = strlen(sqlcmd);
	if(len > DB_PACKET_PAYLOAD_SZ) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"Too long SQL command\n");
		return NULL;
	}

	/* packet creation */
	packet = (db_net_packet_t *)db_mem_alloc(sizeof(db_net_packet_t));
	if(!packet) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::db_mem_alloc(db_net_packet_t)::error\n");
		return NULL;
	}

	/* filling up header */
	packet->cmd = htonl(DBMAN_CLIENT_CMD_SQL);
	packet->xid = htonl(xid);
	packet->len = htonl(len);
	memset(packet->payload,0x00,DB_PACKET_PAYLOAD_SZ);     /* zeroing... */
	memcpy(packet->payload,sqlcmd,len);

	/* packet creation */
	network_packet = net_packet_make(
		client->channel,
		DB_SERVER_CHANNEL_NUMBER,
		0,         /* message type */
		sizeof(db_net_packet_t),
		0,         /* option */
		(char *)packet);
	if(!network_packet) {
		db_mem_free(packet);
		dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::net_packet_make::error\n");
		return NULL;
	}

	/* transmitting a packet */
	if(net_packet_send(client->channel,network_packet) == ERROR) {
		net_packet_free(client->channel,network_packet);
		dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::net_packet_send::error\n");
		return NULL;
	}

	/* deleting a packet transmitted */
	if(net_packet_free(client->channel,network_packet) == ERROR) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::net_packet_free::error\n");
		return NULL;
	}

	network_packet = NULL;

	/* catching up a reply */
	network_packet = net_packet_recv(client->channel,0);
	if(!network_packet) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::net_packet_recv::error\n");
		return NULL;
	}

	/* checking payload */
	if(!NET_PACKET_PAYLOAD(network_packet)) {
		db_mem_free(network_packet);
		dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::net_packet_recv::NULL payload\n");
		return NULL;
	}

	/* generating result list */
	result = (db_result_list *)dbman_sql_decrypt_result(
		NET_PACKET_PAYLOAD(network_packet),
		DB_PACKET_PAYLOAD_SZ);
	if(!result) {
		net_packet_free(client->channel,network_packet);
		dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::net_packet_recv::NULL payload\n");
		return NULL;
	}

	/* cleaning up network packet */
	if(net_packet_free(client->channel,network_packet) == ERROR) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"..run_sql_command::net_packet_free::error\n");
		return NULL;
	}

	/* successfully done */
	dbman_printf(DBMAN_DBG_LVL_MSG,"..run_sql_command(%s)::done\n",sqlcmd);
	return result;
}

/*
 * Traversing...
 *
 */
int dbman_result_traverse(db_result_list *result,dbman_traverse_function_t func)
{
	int ix,iy;
	unsigned int row,column;
	db_result_list_column  *line;
	db_result_list_item    *item;

	if(!result) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"result_traverse::NULL result list\n");
		return (-1);
	}

	row = dbman_result_list_size(result);     /* ROW */
	for(ix=0; ix<row; ix++) {
		line = dbman_fetch_column(result,ix);
		if(!line) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"result_traverse::NULL result column list at %d\n",ix);
			return (-1);
		}
		column = dbman_result_column_size(line);
		for(iy=0; iy<column; iy++) {
			item = dbman_fetch_item(line,iy);
			if(!item) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"result_traverse::NULL result item at %d\n",iy);
				return (-1);
			}
			switch(DB_ITEM_TYPE(item)) {
			case DB_TYPE_INTEGER:
				if(func) func(DB_TYPE_INTEGER,ix,iy,
					          (void *)&(item->intval),sizeof(unsigned int));
				break;
			case DB_TYPE_STRING:
				if(func) func(DB_TYPE_STRING,ix,iy,
					          (void *)(item->strval),item->lenstr);
				break;
			default:
				dbman_printf(DBMAN_DBG_LVL_ERR,"result_traverse::unknown type\n");
				break;
			}

		}
	}

	return 0;
}


