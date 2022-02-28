#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fuse.h>

#include "transport.h"
#include "net.h"

using namespace std;

//
//
//
// P R I V A T E     I N T E R F A C E S 
//
//
//


//
//
//
// P U B L I C       I N T E R F A C E S 
//
//
//

int lpps_net_send( uint8_t id, uint8_t type, const char * msg )
{
	lpps_pkt_t pkt;
	int len = 0;

	if (!msg)
		return -1;

	len = strlen( msg );
	pkt.hdr.len = htonl( len );
	pkt.hdr.sid = id;
	strncpy( (char *)&(pkt.data), msg, PAYLOAD_LENGTH );

	return transport_send( lpps.net.network , lpps.net.fd , type , pkt );
}

