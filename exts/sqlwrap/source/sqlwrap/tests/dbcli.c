#include <stdio.h>
#include <stdlib.h>
#include "oskit.h"
#include "dblib.h"

static unsigned char table_name[100];
static unsigned char db_sql_statement[512];
static db_client_t *instance;
static db_result_list *db_result;
static int base_index;

/* traverse function */
static void cli_result_traverse(
	const int type, const int row, const int col, const void *arg,const int sz)
{
	printf("Col=(%5d),Row=(%5d)\n",col,row);
	if(col==0) printf("\n");
	switch(type) {
	case DB_TYPE_INTEGER:
		printf("[value=%d, sz=%d]",*(unsigned int *)arg,sz);
		break;
	case DB_TYPE_STRING:
		printf("[value=%s, sz=%d]",(unsigned char *)arg,sz);
		break;
	}
}

int db_client_main(void *arg)
{
	int ii;

	while(1) {

		/* thread create */
		instance = dbman_connect_service((127<<24)|1,1092,NULL);
		if(!instance) {
			printf("DB manager connection::failed\n");
			return;
		}

		sprintf(table_name,"test%dinfo",base_index);

		/* Creating a table */
		sprintf(db_sql_statement,"create table %s(id int primary key,name varchar(30),nick varchar(16))",table_name);
		db_result = dbman_run_sql_command(instance, 0x9872, db_sql_statement);
		if(!db_result) {
			printf("Error in executing SQL command\n");
		}
		if(dbman_result_error(db_result)) {
			printf("sql(%s) produces an error = [%s]\n",db_sql_statement,dbman_result_errmsg(db_result));
		}
		dbman_result_free(db_result);

		for(ii=0; ii<4; ii++) {
			/* Request command... */
			memset(db_sql_statement,0,512);
			sprintf(db_sql_statement,"insert into %s values(%d,'hello','logging')",table_name,ii+10+base_index);
			db_result = dbman_run_sql_command(instance, 0x9878, db_sql_statement);
			if(!db_result) {
				printf("Error in executing SQL command\n");
			}
			if(dbman_result_error(db_result)) {
				printf("sql(%s) produces an error = [%s]\n",db_sql_statement,dbman_result_errmsg(db_result));
			}
			dbman_result_free(db_result);
		}

		/* retrieving data */
		sprintf(db_sql_statement,"select * from %s",table_name);
		db_result = dbman_run_sql_command(instance, 0x9878, db_sql_statement);
		if(!db_result) {
			printf("Error in executing SQL command\n");
		}
		/* traversing... */
		dbman_result_traverse(db_result,(dbman_traverse_function_t)cli_result_traverse);

		dbman_result_free(db_result);

		/* removing all data */
		sprintf(db_sql_statement,"delete from %s",table_name);
		db_result = dbman_run_sql_command(instance, 0x9878, db_sql_statement);
		if(!db_result) {
			printf("Error in executing SQL command\n");
		}
		dbman_result_free(db_result);

		/* deleting table */
		sprintf(db_sql_statement,"drop table %s",table_name);
		db_result = dbman_run_sql_command(instance, 0x9878, db_sql_statement);
		if(!db_result) {
			printf("Error in executing SQL command\n");
		}
		dbman_result_free(db_result);

		dbman_disconnect_service(instance);

		printf("\n\n\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n\n");

		os_thread_sleep(100000);

	} /* while(1) */
}

main(int argc,char *argv[])
{
	unsigned int thr;

	os_init(); /* Base component initialize... */

	base_index = atoi(argv[1]);

	dbman_service_initialize(0,NULL,NULL);

	thr = os_thread_create("DBcli",
	                       (os_thrfunc_t)db_client_main,
	                       NULL,NULL,0);

	while(1) {
		//printf("\n==================================\n");
		//os_cond_verbose();
		//buff_verbose(-1);
		sleep(1);
		//printf("\n==================================\n");
	}
}
