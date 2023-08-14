#ifndef __SERVICE_PROC_HEADER__
#define __SERVICE_PROC_HEADER__

#define EXECUTOR	"/sbin/xcfgcli.sh"
#define TMPPATH		"/var/tmp"
#define DELIMITER	'#'
#define CHARSET		"charset="
#define JSON_STARTSTR	"Content-type: application/json\r\n\r\n"
#define JSON_STARTLEN	strlen(JSON_STARTSTR)
#define CMDSTR		"Command="
#define PATHSTR		"Path="
#define XCFGD_CMDLEN	256

/*
 * 
 * M A C R O s 
 *
 */
#define SVCFUNC_NAME(name)	proc_##name
#define SVCFUNC_BEGIN(name,len)  \
		int proc_##name(int mode, char * path, char *payload) {        \
			int jret = 0;                             \
			char jstr[ len + JSON_STARTLEN ];         \
			char cmd[ XCFGD_CMDLEN ];

#define SVCFUNC_CLEAR(len)	\
			{memset( jstr, 0, len + JSON_STARTLEN );  \
			strcpy( jstr, JSON_STARTSTR );            \
			memset( cmd, 0, XCFGD_CMDLEN );}

#define SVCFUNC_PUTCMD()	\
			{snprintf(cmd, XCFGD_CMDLEN-6, "put %s",path);}

#define SVCFUNC_GETCMD()	\
			{snprintf(cmd, XCFGD_CMDLEN-6, "get %s",path);}

#define SVCFUNC_CONTENT(x)	\
			(char *)&(jstr[ JSON_STARTLEN ])

#define SVCFUNC_CONTENTy(x)	\
			(char *)&(jstr[ JSON_STARTLEN + x ])

#define SVCFUNC_END		\
		fprintf(stdout, jstr);                           \
		return jret; }

/*
 *
 * T Y P E s
 *
 */
typedef int (*svc_proc_t)(int, char *, char *);
typedef struct {
	char *cmd;
	int (*proc)(int, char *, char *);
} cmdtbl_t;
extern cmdtbl_t cmdtbl[];

/*
 *
 * F U N C T I O N s
 *
 */

#endif /* __SERVICE_PROC_HEADER__ */
