#!/bin/bash

usage() {
	echo "xcfgcli.sh [get/put] [node] [<value>]"
	exit
}

rand=`echo $RANDOM`
TEMPFILE=/var/tmp/xcfgcli_temp___$rand.file

##
## Please refer to PORT_FILE definition in common/common.h 
##

MYSELF="xcfgd.cfg"
pid=`pidof $MYSELF`
PORTFILE=/var/tmp/$pid/port.info

[ -f $PORTFILE ] || exit

KEYFILE="/var/tmp/key.pem"
CAFILE="/var/tmp/cert.pem"

port=`cat $PORTFILE`
cmd=$1
node=$2

case $cmd in
  "get") 
	if [ "$node" = "" ]; then usage; fi
	curl -s -H "Content-Type: application/json" -X GET http://127.0.0.1:$port/$node > $TEMPFILE 
	;;
  "xget")
	if [ "$node" = "" ]; then usage; fi
	curl --key $KEYFILE --cacert $CAFILE -s -H "Content-Type: application/json" -X GET https://127.0.0.1:$port/$node > $TEMPFILE
	;;
  "put") 
	value=$3
	if [ "$value" = "" ]; then usage; fi
	fnode=${node##*/}
	payload="{\"$fnode\": \"$value\"}"
	curl -s -H "Content-Type: application/json" -X PUT http://127.0.0.1:$port/$node -d "$payload" > $TEMPFILE 
	;;
  "xput")
	value=$3
	if [ "$value" = "" ]; then usage; fi
	fnode=${node##*/}
	payload="{\"$fnode\": \"$value\"}"
	curl --key $KEYFILE --cacert $CAFILE -s -H "Content-Type: application/json" -X PUT https://127.0.0.1:$port/$node -d "$payload" > $TEMPFILE
	;;
  *) 
	usage
	;;
esac

result=`cat $TEMPFILE`
rm -rf $TEMPFILE

##
## Check if it is tree or element 
##
temp=${result%,*}
if [ "$temp" = "$result" ]; then  
	id=`echo $result | awk '{print $1}'`
	fnode=${result#$id}
	fnode=${fnode:2}
	fnode=${fnode%\"*}
	echo $fnode
else
	echo $result
fi
