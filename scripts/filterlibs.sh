#!/bin/sh

find $1 -name "*.pc" -exec grep -H Libs: {} \; | awk '{print $3 $4 $5 $6 $7}' | sed -e "s/-[^l].*//g" 

