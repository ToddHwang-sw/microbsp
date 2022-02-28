#ifndef CONFIGURATION_FILE_HEADER
#define CONFIGURATION_FILE_HEADER

#define CONF_MEMSPC	"memory"
#define CONF_MEMBOT	"bmemory"
#define CONF_JSONBUF	"jsonbuf"
#define CONF_DEBUG	"debug"
#define CONF_FILESIZE	"esize"
#define CONF_MAXSHARE	"maxshare"
#define CONF_FUSEBK	"fusebackground"
#define CONF_MEMTRACE	"memtrace"
#define CONF_ACCTRACE	"acctrace"
#define CONF_MRTIMEOUT	"mrtimeout"
#define CONF_MAXTASK	"maxtask"
#define CONF_HISTSIZE	"history"

#define CONF_NETWORK	"network"
#define CONF_NETIP	"ip"
#define CONF_NETPORT	"port"
#define CONF_TRANSPORT	"transport"
#define CONF_NETID	"netid"
#define CONF_NETDEV	"netdev"

//
//
// configuration structure 
//
//
typedef struct __conf_t{
	char *sym;
	union {
		int dnum;
		float fnum;
		char *sval;
	}v;
	struct __conf_t *next;
}conf_t;

//
//
// P U B L I C    F U N C T I O N S 
//
//
extern conf_t * conf_read( const char *fn );
extern int      conf_ival( conf_t * , const char *sym );
extern float    conf_fval( conf_t * , const char *sym );
extern char *   conf_sval( conf_t * , const char *sym );

#endif
