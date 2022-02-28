#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "conf.h"
#include "os.h"

//
// Notice: We do not use memory abstraction layer supporting such as "os_malloc" or "os_free". 
// This is called ahead of the initiation of memory abstraction layer. 
// Definitely, it even includes the maximum size of memory dedicated to the abstraction layer,
// we should implement all functions with UNIX standard functions. 
//

//
// C++
//
using namespace std;

//
//
//
// P R I V A T E     F U N C T I O N S 
//
//
//
static char *str_pack( char *str )  // string compaction
{
	#define skippable(x) \
		(((x)==0x0) || ((x)=='"') || ((x)=='\n') || ((x)=='\t') || ((x)==' '))

	// beginning 
	char *f = str;
	while (skippable(*f))
		++f;

	// ending 
	char *e = str+strlen(str);
	while (skippable(*e))
		*e-- = 0;
		
	return f;
}

static conf_t *conf_new_node( char *sym )
{
	conf_t * temp;

	// allocate 
	temp = (conf_t *)malloc( sizeof(conf_t) );
	if (!temp) {
		ERR(" CONF::malloc( sizeof(conf_t) ) failed\n");
		return NULL;
	}

	// symbol duplication 
	temp->sym = strdup(sym);
	if (!temp->sym) {
		ERR(" CONF::strdup(symbol) failed\n");
		free( temp );
		return NULL;
	}

	return temp;
}

static conf_t *conf_find_node( conf_t *root, const char *sym )
{
	conf_t * temp;

	if (!root || !sym)
		return NULL;

	temp = root;
	while (temp) {
		if (!strncmp(temp->sym,sym,strlen(sym)))
			return temp;
		temp = temp->next;
	}

	return NULL;
}

static void conf_free_node(conf_t *node) 
{
	free(node->sym);
	free(node);
}

//
//
//
// P U B L I C       F U N C T I O N S 
//
//
//
conf_t * conf_read( const char *fn )
{
	conf_t *root = NULL;
	conf_t *temp;
	FILE *fp;
	char *sym, *val;

	#define BUFSZ 1024
	static char buf[BUFSZ];

	fp = fopen(fn,"r+");
	if (!fp) {
		ERR(" configuration file <%s> open error\n",fn);
		return NULL;
	}

	#define SYMBOL(NAME)	(!strncmp(sym,NAME,strlen(NAME)))

	//
	// parsing the configuration and loading it into memory
	//
	while (!feof(fp)) {
		memset(buf,0,BUFSZ);
		if(fgets(buf,BUFSZ-1,fp) == NULL)
			break;

		if (buf[0] == '#') // check comment 
			continue;

		if (buf[0] == '\n') // check blank line
			continue;

		// tokenizing 
		sym = strtok(buf,"=");
		if (!sym)
			continue;

		val = strtok(NULL,"\n");
		if (!val)
			continue;

		// compaction 
		sym = str_pack( sym );
		val = str_pack( val );

		// make a node 
		temp = conf_new_node(sym);
		if (!temp) 
			continue;

#define SETUP_INTEGER(sym) \
		if (SYMBOL(CONF_##sym)) { \
			temp->v.dnum = atoi(val); \
		}else

#define SETUP_STRING(sym) \
		if (SYMBOL(CONF_##sym)) { \
			temp->v.sval = strdup(val); \
		}else

		SETUP_INTEGER( MEMSPC   )  // Mbytes 
		SETUP_INTEGER( MEMBOT   )  // Kbytes 
		SETUP_INTEGER( JSONBUF  )  // Kbytes 
		SETUP_INTEGER( DEBUG    )  // 0/1 
		SETUP_INTEGER( FILESIZE )  // filesize expansion multiplier 
		SETUP_INTEGER( MAXSHARE )  // max share 
		SETUP_INTEGER( FUSEBK   )  // libfuse background process
		SETUP_STRING ( MEMTRACE )  // memory trace file name 
		SETUP_STRING ( ACCTRACE )  // access trace file name 
		SETUP_INTEGER( MRTIMEOUT)  // multiple reader timeout
		SETUP_INTEGER( MAXTASK  )  // max reader/write tasks
		SETUP_INTEGER( HISTSIZE )  // history size
		SETUP_INTEGER( NETWORK  )  // multicast broadcast ??
		SETUP_STRING ( NETIP    )  // multicast IP ??
		SETUP_INTEGER( NETPORT  )  // multicast UDP port 
		SETUP_STRING ( TRANSPORT)  // udpv4/udpv6/raw
		SETUP_INTEGER( NETID    )  // network ID
		SETUP_STRING ( NETDEV   )  // network device
		{
			ERR(" CONF::unknown symbol %s \n",sym);
			// delete a node 
			conf_free_node(temp);
			continue;
		}

		// push back 
		temp->next = root;
		root = temp;
	}

	fclose(fp);

	return root;
}

int conf_ival( conf_t * conf , const char *sym )
{
	conf_t *node = conf_find_node( conf, sym );

	return node ? node->v.dnum : -1;
}

float conf_fval( conf_t * conf , const char *sym )
{
	conf_t *node = conf_find_node( conf, sym );

	return node ? node->v.fnum : -0.1;
}

char * conf_sval( conf_t * conf , const char *sym )
{
	conf_t *node = conf_find_node( conf, sym );

	return node ? node->v.sval : NULL;
}

