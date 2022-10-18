#!/bin/sh

executed=`ps -aef | grep lighttpd | wc -l`
[ "$executed" = "0" ] || killall lighttpd
WEBTMPDIR=/var/tmp/lighttpd
[ ! -d $WEBTMPDIR ] || \rm -rf $WEBTMPDIR
mkdir -p $WEBTMPDIR
mkdir -p $WEBTMPDIR/docs
mkdir -p $WEBTMPDIR/logs
mkdir -p $WEBTMPDIR/error
mkdir -p $WEBTMPDIR/uploads
mkdir -p $WEBTMPDIR/compress

## change rmem paramtere
echo 2097152 > /proc/sys/net/core/rmem_default
echo 2097152 > /proc/sys/net/core/rmem_max
## HTTP server ....
lighttpd -f lighttpd.conf 
