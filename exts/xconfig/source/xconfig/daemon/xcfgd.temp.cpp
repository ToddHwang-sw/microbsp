#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>
#include <pthread.h>
#include "scew.h"
#include "dmt.h"
#include "jansson.h"
#include "malloc.h"
#include "xmlmem.h"
#include "common.h"
#include "microhttpd.h"
#include "xml.h"

/*
 *
 *
 * P R I V A T E    D E F I N I T I O N S
 *
 *
 */

/* server info */
struct xcfg_info {

	/* server socket */
	int sock;               /* main socket */
	#define DEFAULT_XCFG_PORT 54321
	unsigned short port;    /* TCP port */
	struct sockaddr_in saddr; /* server listen address */
	struct sockaddr_in paddr; /* accepted client peer address */
	int paddrlen;             /* address length of accepted client */
	int done;               /* termination flag */

	/* HTTP connection */
	#define DEFAULT_HTTP_PORT 8083
	unsigned short http_port; /* HTTP port */
	struct MHD_Daemon *httpd; /* HTTP daemon */

	/* client information */
	#define MAXCLIENT   8
	struct client_info clients[ MAXCLIENT ];
	int client_timeout;

	/* XML information */
	char path[ PATH_MAX + 8 ];
	scew_parser     *parser;
	scew_tree       *tree;

	/* temporary data for XML handling */
	scew_list       *list;
	scew_element    *element;
	char            *contents;

	/* json Managament */
	json_t          *json_res;

} __attribute__((aligned));

/*
 *
 *
 * P R I V A T E    V A R I A B L E S
 *
 *
 */

static struct xcfg_info * xcfg;

/*
 *
 *
 * P R I V A T E    F U N C T I O N S
 *
 *
 */

/*
 * building up TCP server socket
 */
static int build_tcp_socket(struct xcfg_info * xmli)
{
	int rc;
	int opt = 1; /* enable option */

	/* create a socket instance */
	xmli->sock = socket( AF_INET, SOCK_STREAM, 0 );
	if (xmli->sock <= 0) {
		ERR("socket( AF_INET )::failed\n");
		return -1;
	}

	/* SYS TCP port */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/sysport" );
	if( xcfg->json_res ) {
		char _sport[32];
		xmli->port =
			!config_xml_json_value( xcfg->json_res, _sport ) ? atoi(_sport) : DEFAULT_XCFG_PORT;
		config_xml_remove( xcfg->json_res );
	}
	DBG("TCP PORT = %d\n",xmli->port);

	/* address information */
	memset( (char *)&xmli->saddr, 0, sizeof(struct sockaddr_in) );
	xmli->saddr.sin_family = AF_INET;
	xmli->saddr.sin_addr.s_addr = INADDR_ANY;
	xmli->saddr.sin_port = htons( xmli->port );

	/* socket option flag */
	rc = setsockopt( xmli->sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
	                 &opt, sizeof(opt) );
	if (rc != 0) {
		ERR("setsockopt( SO_REUSEADDR|SO_REUSEPORT )::failed\n");
		goto __exit_tcp_sock;
	}

	/* bind to a port */
	rc = bind( xmli->sock, (struct sockaddr *)&xmli->saddr,
	           sizeof(struct sockaddr) );
	if (rc != 0) {
		ERR("bind( to a port %d )::failed\n",xmli->port);
		goto __exit_tcp_sock;
	}

	/* setup concurrent accpetance */
	listen( xmli->sock, 5 );

	return 0;

__exit_tcp_sock:
	close(xmli->sock);
	return -1;
}

static int allocate_client(struct xcfg_info *xcfg)
{
	int ix;

	for(ix = 0; ix < MAXCLIENT; ix++) {
		if( xcfg->clients[ix].used )
			continue;
		xcfg->clients[ ix ].used = 1;
		return ix;
	}
	return -1;
}

static __inline__ void cleanup_single_client( struct client_info *cli )
{
	if (!cli)
		return;

	if (!(cli->used && !cli->done))
		return;

	cli->done = 1; /* terminating signal */
	pthread_join( cli->thr, NULL );
	/* clean up threads - it might take at least CLIENT_TIMEOUT second */
	close( cli->csock ); /* shutdown socket */
	free( cli->n.pkt );  /* packet freeing  */
	cli->used = 0;       /* unused */
	DBG("CLIENT[ %s:%d ] terminated\n",
	    inet_ntoa( cli->paddr.sin_addr ), ntohs( cli->paddr.sin_port ) );
}

/* transmit function in network - works in non-blocking mode?? */
static __inline__ int _write( int sock, char *buf, int sz )
{
	int rc;
	int totsz = 0;

	while( totsz < sz ) {
		rc = write( sock, buf+totsz, sz-totsz );
		if (rc < 0) {
			ERR("write( %d/%d )::failed\n",sz-totsz,sz);
			return -1;
		}
		totsz += rc;
	}
	return totsz;
}

static void * client_service( void * arg )
{
	struct client_info *cli = (struct client_info *)arg;
	int rc;
	int rbytes; /* amount of data received */
	struct timeval tv;
	short pktlen;
	unsigned int magic;
	struct jxmlpkt *jpkt;

	if (!cli)
		return NULL;

	while( !cli->done ) {
		/* ready for multiplexing */
		FD_ZERO(&cli->rfds);
		FD_SET(cli->csock,&cli->rfds);

		/* second granularity */
		tv.tv_sec = cli->timeout;
		tv.tv_usec = 0;

		rbytes = 0;
		rc = select(cli->csock+1,&cli->rfds,NULL,NULL,&tv); /* multiplexing */
		if (rc < 0) {
			ERR("client error \n");
			cli->done = 1; /* terminate */
			continue;
		}else if (rc == 0) {
			DBG("client timeout \n"); /* timeout */
			continue;
		}

		/* normal packet receive */
		if (!FD_ISSET(cli->csock,&cli->rfds)) {
			ERR("CLIENT[ %s:%d ] select error\n",
			    inet_ntoa( cli->paddr.sin_addr ), ntohs( cli->paddr.sin_port ) );
			cli->done = 1;
			continue;
		}

		/* TCP receice */
		rbytes = read(cli->csock, cli->n.pkt + cli->n.cur, cli->n.trg - cli->n.cur );
		if (rbytes <= 0) {
			ERR("CLIENT[ %s:%d] read(%d) rbytes=%d\n",
			    inet_ntoa( cli->paddr.sin_addr ), ntohs( cli->paddr.sin_port ),
			    cli->csock, rbytes );
			cli->done = 1;
			continue;
		}

		cli->n.cur += rbytes;
		if (cli->n.cur < cli->n.trg)
			continue;

		DBG("CLIENT[ %s:%d ] %d %d/%d --> RX DONE\n",
		    inet_ntoa( cli->paddr.sin_addr ), ntohs( cli->paddr.sin_port ),
		    rbytes, cli->n.cur, cli->n.trg);

		/* totally received */
		switch (cli->n.ishd) {
		case 1:
			/* check header */
			jpkt = (struct jxmlpkt *)cli->n.pkt;
			/* 1. magic code */
			magic =  ntohl( jpkt->h.magic  );
			if (magic != JPKT_MAGIC) {
				ERR("CLIENT read(%d) rbytes=%d::wrong magic\n",cli->csock,rbytes);
				continue;
			}
			/* 2. packet length */
			pktlen = ntohs( jpkt->h.length );
			if ((pktlen < 0) || (pktlen > MAXPKTLEN)) {
				ERR("CLIENT read(%d) header length field < 0\n",cli->csock);
				continue;
			}
			/* depending on payload length, next phase might be either hader or payload */
			if (pktlen == 0) {
				cli->n.trg = sizeof(struct jxmlpkt);
				cli->n.cur = 0;
				DBG("CLIENT[ %s:%d ] --> HEADER RECEIVE \n",
				    inet_ntoa( cli->paddr.sin_addr ), ntohs( cli->paddr.sin_port ));
				cli->n.ishd = 1; /* head again */
			} else {
				cli->n.trg += pktlen;
				cli->n.ishd = 0; /* payload */
				DBG("CLIENT[ %s:%d ] --> PAYLOAD (%d+%d=%d) RECEIVE \n",
				    inet_ntoa( cli->paddr.sin_addr ), ntohs( cli->paddr.sin_port ),
				    cli->n.cur, pktlen, cli->n.trg );
			}
			break;
		case 0:
			/* PAYLOAD */
			cli->n.trg = sizeof(struct jxmlpkt);
			cli->n.cur = 0;
			cli->n.ishd = 1;
			DBG("CLIENT[ %s:%d ] --> HEADER RECEIVE \n",
			    inet_ntoa( cli->paddr.sin_addr ), ntohs( cli->paddr.sin_port ));
			break;
		default:
			ERR("CLIENT[ %s:%d ] UNKNOWN RECEIVE MODE\n",
			    inet_ntoa( cli->paddr.sin_addr ), ntohs( cli->paddr.sin_port ));
			cli->done = 1;
			break;
		} /* case... */
	} /* while( ... */

	return NULL;
}

static void cleanup_xcfg( struct xcfg_info *xcfg )
{
	int ix;
	struct client_info *cli;

	/* delete communication socket */
	/*
	 * Waiting client thread on select() will be freed up after timeout
	 */
	for( ix = 0; ix < MAXCLIENT; ix++ ) {
		cli = (struct client_info *)&(xcfg->clients[ ix ]);
		cleanup_single_client( cli );
	}
}


/*
 *
 *
 * H T T P    S E R V E R    C A L L B A C K S
 *
 *
 */
static int AcceptCallback(void * cls,
                          const struct sockaddr *addr,
                          unsigned int len)
{
	struct sockaddr_in a;

	/*
	 * "cls" is NULL !!
	 */

	/* make temporal copy */
	memset((char *)&a, 0, sizeof(struct sockaddr_in));
	memcpy((char *)&a, (char *)addr, sizeof(struct sockaddr_in));

	DBG("HTTP ACCEPT [CLIENT] %s\n",inet_ntoa( a.sin_addr ) );

	return MHD_YES;
}

static int DefaultHandlerCallback(void *cls,
                                  struct MHD_Connection *connection,
                                  const char *url,
                                  const char *method,
                                  const char *version,
                                  const char *updata, size_t *updata_size,
                                  void **unused)
{
	struct MHD_Response *response = NULL;
	unsigned int status_code = 200; /* OK */
	const char *purl = url+1;
	char * respmesg;

	DBG("HTTP HANDLE URL=%s METHOD=%s VER=%s\n", url, method, version );
	if (updata)
		DBG("HTTP HANDLE DATA=%s\n", updata);

	if (strcasecmp(method,"GET") == 0) {
		/* retrieve value */
		xcfg->json_res = config_xml_retrieve( xcfg->tree, purl );
		if( !xcfg->json_res ) {
			ERR("HTTP URL(%s) is unknown\n",url);
			status_code = 302; /* URI not found */
			respmesg = (char *) os_malloc( strlen("{\"error\"}") );
		}
		respmesg =
			json_dumps( xcfg->json_res, JSON_PRESERVE_ORDER );
		DBG("HTTP URL RES=(%s)\n",respmesg);
		if (!respmesg)
			return MHD_NO;
	} else
	if ((strcasecmp(method,"POST") == 0) ||
	    (strcasecmp(method,"PUT") ==0)) {
		json_t *jobj = NULL;

		if (!strlen(updata)) {
			ERR("HTTP PUT/POST NULL VALUE\n");
			return MHD_NO;
		}

		/* moving to json object */
		#define _TEMP_JSON_FILE_ "/var/tmp/__json_file.temp"
		FILE * fp = (FILE *)fopen(_TEMP_JSON_FILE_,"w+");
		if (!fp) {
			ERR("json temp file (%s) creation error\n",_TEMP_JSON_FILE_);
			status_code = 302;
			respmesg = (char *) os_malloc( strlen("{\"error\"}") );
		} else {
			fprintf(fp,"%s",updata);
			fclose(fp);
			json_error_t jerr;
			jobj = json_load_file(_TEMP_JSON_FILE_,0,&jerr);
			if (!jobj) {
				ERR("json object cannot be built\n");
				status_code = 302; /* URI not found */
				respmesg = (char *) os_malloc( strlen("{\"error\"}") );
			}
			unlink( _TEMP_JSON_FILE_ );
		}

		if (status_code == 200) {
			char newval[ 128 ];
			if (!config_xml_json_value( jobj, newval )) {
				xcfg->json_res = config_xml_set( xcfg->tree,
				                                 (const char *)purl, (const char *)newval );
				/* update value */
				respmesg = json_dumps( jobj, JSON_PRESERVE_ORDER );
			}
		}
		DBG("HTTP URL RES=(%s)\n",respmesg);
		if( jobj )
			json_decref( jobj );

	} /* POST/PUT */

	/*
	 * NOTICE - string shorter than 64 has been processed
	 * with scrambled garbage - probably fault in HTTP library.
	 */
	if (strlen(respmesg) < 64)
		respmesg = (char *)os_realloc(respmesg,64);

	/* response message */
	response = MHD_create_response_from_buffer(
		strlen(respmesg), (void *)respmesg, MHD_RESPMEM_PERSISTENT);
	MHD_queue_response( connection, status_code, response );

	/* remove json */
	config_xml_remove( xcfg->json_res );
	json_decref( xcfg->json_res );

	/* remove response */
	if( respmesg )
		os_free( respmesg );

	/* delete response */
	if( response )
		MHD_destroy_response(response);

	return MHD_YES;
}

static void ConnectionCallback(void * cls,
                               struct MHD_Connection *connection,
                               void **socket_context,
                               enum MHD_ConnectionNotificationCode toe)
{
	static int started = MHD_NO;

	switch (toe)
	{
	case MHD_CONNECTION_NOTIFY_STARTED:
		DBG("HTTP CONNECTION START\n");
		if (MHD_NO != started)
			return;
		started = MHD_YES;
		*socket_context = &started;
		break;
	case MHD_CONNECTION_NOTIFY_CLOSED:
		DBG("HTTP CONNECTION CLOSE\n");
		if (MHD_YES != started)
			return;
		if (&started != *socket_context)
			return;
		*socket_context = NULL;
		started = MHD_NO;
		break;
	default:
		DBG("HTTP CONNECTION UNKNOWN\n");
		break;
	}
}

static void * LogCallback(void *cls,
                          const char *uri,
                          struct MHD_Connection *connection)
{
	if (uri)
		DBG("HTTP URI %s \n",uri);

	return (void *)NULL;
}

static void CompletionCallback(void *cls,
                               struct MHD_Connection *connection,
                               void **con_cls,
                               enum MHD_RequestTerminationCode toe)
{
	DBG("HTTP COMPLETION\n");

	if ( (toe != MHD_REQUEST_TERMINATED_COMPLETED_OK) &&
	     (toe != MHD_REQUEST_TERMINATED_CLIENT_ABORT) &&
	     (toe != MHD_REQUEST_TERMINATED_DAEMON_SHUTDOWN) )
		return;

	__os_free (*con_cls);
	*con_cls = NULL;
}

/*
 *
 *
 * M A I N          F U N C T I O N S
 *
 *
 */

int main(int argc,char *argv[])
{
	int ret;

	/* option processsing */

	xcfg = (struct xcfg_info *)malloc( sizeof(struct xcfg_info) );
	if (!xcfg) {
		ERR("malloc( sizeof(struct xcfg_info) )::failed\n");
		return 1;
	}

	memset( (char *)xcfg, 0, sizeof(struct xcfg_info) );
	strcpy( xcfg->path, "./config.xml" );
	xcfg->parser = NULL;
	xcfg->tree =   NULL;
	xcfg->list =   NULL;
	xcfg->element =  NULL;
	xcfg->contents = NULL;
	xcfg->done = 0;

	/* make XML trees get ready */
	ret = config_xml_init( xcfg->path, &xcfg->parser, &xcfg->tree );
	if ( ret < 0 ) {
		ERR("config_xml_init -> failed \n");
		return 1;
	}
	DBG("config_xml_init::done\n");

	/* HTTP port */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/httpport" );
	if( xcfg->json_res ) {
		char _hport[32];
		xcfg->http_port =
			!config_xml_json_value( xcfg->json_res, _hport ) ? atoi(_hport) : DEFAULT_HTTP_PORT;
		config_xml_remove( xcfg->json_res );
	}
	DBG("HTTP PORT : %d\n", xcfg->http_port);

	/* TIMEOUT */
	xcfg->json_res = config_xml_retrieve( xcfg->tree, (const char *)"xcfgd/timeout" );
	if( xcfg->json_res ) {
		char _timeout[32];
		xcfg->client_timeout =
			!config_xml_json_value( xcfg->json_res, _timeout ) ? atoi(_timeout) : DEFAULT_CLIENT_TIMEOUT;
		config_xml_remove( xcfg->json_res );
	}
	DBG("CLIENT TIMEOUT : %d\n", xcfg->client_timeout);

	xcfg->httpd =
		MHD_start_daemon(
			MHD_USE_THREAD_PER_CONNECTION |
			MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ITC | MHD_USE_ERROR_LOG,
			xcfg->http_port,
			AcceptCallback,                 NULL,
			DefaultHandlerCallback,         NULL,
			MHD_OPTION_URI_LOG_CALLBACK,    LogCallback, NULL,
			MHD_OPTION_NOTIFY_COMPLETED,    CompletionCallback, NULL,
			MHD_OPTION_NOTIFY_CONNECTION,   ConnectionCallback, NULL,
			MHD_OPTION_CONNECTION_TIMEOUT,  (unsigned int)xcfg->client_timeout,
			MHD_OPTION_END );

	/* init */
	/* build up TCP socket and receiving objects */
	if ( build_tcp_socket( xcfg ) ) {
		ERR("build_tcp_socket -> failed \n");
		return 1;
	}
	DBG("build_tcp_socket( socket=%d / port=%d ) \n",xcfg->sock, xcfg->port);

	while( !xcfg->done ) {
		int csock; /* client socket */
		int cindex;
		struct client_info *cli;

		/* wait for a new client */
		memset( (char *)&(xcfg->paddr), 0, sizeof( struct sockaddr_in ) );
		xcfg->paddrlen = sizeof(struct sockaddr_in);
		csock = accept( xcfg->sock, (struct sockaddr *)&(xcfg->paddr), (socklen_t *)&xcfg->paddrlen );
		if (csock < 0) {
			ERR("accept()::failed\n");
			xcfg->done = 1; /* exit */
			continue;
		}

		/* create a client node */
		if( (cindex = allocate_client( xcfg )) < 0 ) {
			ERR("allocate_client::failed\n");
			xcfg->done = 1;
			continue;
		}

		/* client info */
		cli = (struct client_info *)&(xcfg->clients[ cindex ]);
		memcpy( (char *)&(cli->paddr), (char *)&(xcfg->paddr), sizeof( struct sockaddr_in ) );
		cli->paddrlen = xcfg->paddrlen;
		cli->csock = csock;
		cli->timeout = xcfg->client_timeout; /* seconds */
		cli->done = 0;
		cli->n.pkt = (char *)malloc( MAXPKTLEN );
		if( !cli->n.pkt ) {
			ERR("malloc( MAXPKTLEN=%dbytes )::failed\n", MAXPKTLEN);
			xcfg->clients[ cindex ].used = 0; /* simply deallocating the client */
			close( cli->csock );
			continue;
		}

		/* first, get ready for header reception */
		cli->n.trg = sizeof(struct jxmlpkt);
		cli->n.cur = 0;
		cli->n.ishd = 1;

		DBG("CLIENT[ %s:%d ] created\n",
		    inet_ntoa( cli->paddr.sin_addr ), ntohs( cli->paddr.sin_port ) );

		/* change client socket to non-blocking mode */
		/* Q: Really work in TCP connection ?        */
		fcntl( cli->csock, F_SETFL, O_NONBLOCK );

		/* spawn client service thread */
		ret = pthread_create( &(cli->thr), NULL, client_service, (void *)cli );
		if (ret != 0) {
			ERR("pthread_craete::failed\n");
			xcfg->done = 1;
		}

	} /* while( !xcfg->done */

	cleanup_xcfg( xcfg );
}

/*
 *  TODO
 *
 *  1. Signal handling
 *  2. Client elimination at connection close.
 *  3. ....
 *
 */
