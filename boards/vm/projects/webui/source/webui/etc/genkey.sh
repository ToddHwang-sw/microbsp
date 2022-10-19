#!/bin/sh

Path=$1

openssl req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout $Path/key.pem -out $Path/cert.pem -config $Path/req.txt >& /dev/null

