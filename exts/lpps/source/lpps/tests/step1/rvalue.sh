#!/bin/sh

[ "$1" = "" ] || ( cat $1 | grep -a ZZ | cut -d ":" -f 3- )

