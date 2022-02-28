#!/usr/bin/python3

import sys
import getopt
import threading
import pycurl
import json
try:
    from io import BytesIO
except ImportError:
    from StringIO import StringIO as BytesIO

##
## Read configuration example
##
## bash ) 
## curl -s -H "Content-type: application/json" -X GET http://10.10.13.44:8081/lan/ptp/mode 
##
def readcfg(c, ip, port, key):
    try:
        c.setopt(c.VERBOSE,1)
        c.setopt(c.HTTPGET, True)
        c.setopt(c.URL, "http://" + ip + ":" + port + "/" + key )
        c.setopt(c.HTTPHEADER, ['Content-Type: application/json', 'Accept: application/json'])
        response = BytesIO()
        c.setopt(c.CUSTOMREQUEST, "GET")
        c.setopt(c.WRITEDATA, response)
        c.perform()
        c.close()
        body = response.getvalue()
        print ( json.dumps( body.decode('iso-8859-1') , indent=4, separators=(". "," = ") ) )
        return 0
    except pycurl.error:
        print ( "read error !!!" )
        return 1

##
## Write configuration example
##
## bash)
## curl -s -H "Content-type: application/json" -X PUT http://10.10.13.44:8081/lan/ptp/mode -d "{\"mode\": \"slave\"}"
##
def writecfg(c, ip, port, key, value):
    try:
        c.setopt(c.VERBOSE, 1)
        c.setopt(c.URL, "http://" + ip + ":" + port + "/" + key )
        c.setopt(c.HTTPHEADER, ['Content-Type: application/json', 'Accept: application/json'])
        c.setopt(c.CUSTOMREQUEST, "PUT")
        node = key[ key.rindex("/")+1: ]
        response = json.dumps( {node: value} )
        c.setopt(c.POSTFIELDS, response)
        c.perform()
        c.close()
        return 0
    except pycurl.error:
        return 1

##
## READ THREAD FUNCTION
##
def pthread_readcfg(nth,flag,ip,port,key):
    while flag:
        print("------------------------- R E A D ", nth, " -----------------------------");
        res = readcfg( pycurl.Curl() , ip , port , key )
        if res != 0:
            sys.exit(0)
        print( "[RD] Result is " , res )
    return 0

##
## WRITE THREAD FUNCTION
##
def pthread_writecfg(nth,flag,ip,port,key,value):
    while flag:
        print("------------------------- W R I T E ", nth, "---------------------------");
        res = writecfg( pycurl.Curl() , ip , port , key , value )
        if res != 0:
            sys.exit(0)
        print( "[WR] Result is " , res )
    return 0

## template...
unixOptions = "hi:p:r:w:k:v:"
gnuOptions = ["help", "ip=", "port=", "rth=", "wth=", "key=", "value="]

# read commandline arguments, first
fullcmd = sys.argv
argumentList = fullcmd[1:]

try:
    arguments, values = getopt.getopt(argumentList, unixOptions, gnuOptions)
except getopt.error as err:
    print (str(err))
    sys.exit(2)
  
ip="127.0.0.1"
port="8081"
rthnum=0
wthnum=0
key="lan/ptp/mode"
value="unknown"

## arrays for thread ...
rth = []
wth = []

for arg, val in arguments:
    if arg in ("-h", "--help"):
        print(fullcmd[0]+" -h / --help .... help")
        print(fullcmd[0]+" -i / --ip=x..... ip address")
        print(fullcmd[0]+" -p / --port=y... port number")
        print(fullcmd[0]+" -r / --rth=x.... read thread")
        print(fullcmd[0]+" -w / --wth=x.... write thread")
        print(fullcmd[0]+" -k / --key=x.... key (e.g. lan/ptp/mode")
        print(fullcmd[0]+" -v / --value=x.. value of key")
        sys.exit(1)
    elif arg in ("-i", "--ip"):
        ip = val;
    elif arg in ("-p", "--port"):
        port = val;
    elif arg in ("-r", "--rth"):
        rthnum = val;
    elif arg in ("-w", "--wth"):
        wthnum = val;
    elif arg in ("-k", "--key"):
        key = val;
    elif arg in ("-v", "--value"):
        value = val;

doit = 1

##
## Creating thread .....
##
for x in range( int(rthnum) ):
    rth.append( threading.Thread(target=pthread_readcfg, args=(x, doit, ip, port, key,)) )

for x in range( int(wthnum) ):
    wth.append( threading.Thread(target=pthread_writecfg, args=(x, doit, ip, port, key, val,)) )

##
## Activating...
##
for x in range( int(rthnum) ):
    rth[ x ].start()

for x in range( int(wthnum) ):
    wth[ x ].start()


