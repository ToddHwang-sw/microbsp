#include <stdio.h>
#include <stdlib.h>
#include "oskit.h"
#include "dblib.h"

//static unsigned char dbman_dbg_level = DBMAN_DBG_LVL_ERR;
static unsigned char dbman_dbg_level = DBMAN_DBG_LVL_MSG;
static FILE *        dbman_dbg_file_fp;
static unsigned int dbman_dbg_printf_initted = 0;
static unsigned int dbman_dbg_print_lock;
static unsigned char dbman_print_buff[2048]; /* debug buffer.. */

/**************************************************
 *
 * G L O B A L        F U N C T I O N S
 *
 */
unsigned int dbman_printf_init(unsigned char *file_name)
{
	if( dbman_dbg_printf_initted ) {
		printf("already initialized...!!\n");
		return (-1);
	}
	/* if file name specified.. */
	dbman_dbg_file_fp = NULL;
	if(file_name) {
		dbman_dbg_file_fp = fopen(file_name,"w+");
		if(!dbman_dbg_file_fp) {
			printf("failed in opening a file\n");
			dbman_dbg_file_fp = NULL;
		}
	}
	dbman_dbg_print_lock = os_mutex_create("DBdbgPrintfLock");
	if(dbman_dbg_print_lock == ERROR) {
		printf("failed in initializing DBMAN printf\n");
		return (-1);
	}
	dbman_dbg_printf_initted = 1;
	return 0;
}

unsigned int dbman_printf_shutdown()
{
	if(!dbman_dbg_printf_initted) {
		printf("not yet initialized...!!\n");
		return (-1);
	}
	if(dbman_dbg_file_fp) fclose(dbman_dbg_file_fp);
	os_mutex_delete(dbman_dbg_print_lock);
	dbman_dbg_printf_initted = 0;
	return 0;
}

void dbman_printf_level(int lvl)
{
	dbman_dbg_level = lvl;
}

unsigned int dbman_printf(int lvl,const char *fmt, ...)
{
	va_list ap;
	int len;

	if(lvl <= dbman_dbg_level) {

		/* lock */
		if(!dbman_dbg_printf_initted) {
			/* Not yet initialized... */
			va_start(ap,fmt);
			len = sprintf(dbman_print_buff,"(sySBD)");
			len += vsnprintf(dbman_print_buff+len,2048-len,fmt,ap);
			va_end(ap);
			/* printf */
			if(dbman_dbg_file_fp)
				fprintf(dbman_dbg_file_fp,"%s",dbman_print_buff);
			else
				printf("%s",dbman_print_buff);
			return 1;
		}

		os_mutex_lock(dbman_dbg_print_lock,0);

		va_start(ap,fmt);
		len = sprintf(dbman_print_buff,"(DBSys)");
		len += vsnprintf(dbman_print_buff+len,2048-len,fmt,ap);
		va_end(ap);

		/* printf */
		if(dbman_dbg_file_fp)
			fprintf(dbman_dbg_file_fp,"%s",dbman_print_buff);
		else
			printf("%s",dbman_print_buff);

		/* unlock */
		os_mutex_unlock(dbman_dbg_print_lock);
	}
	return 1;
}

