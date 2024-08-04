#!/bin/bash

OPTION=$1
SRCFILE=$2
DSTLOC=$3
SPECBR=$4

if [ "$SRCFILE" == "" ]; then
	echo "No git location " && exit 1
fi

if [ "$DSTLOC" != "" ]; then
	[ -d $DSTLOC ] || mkdir $DSTLOC
fi

## DATE ... 
FINAL_DATE=`cat $TOPDIR/date.inc`

## New folder
[ ! -d ./${DSTLOC} ] || cd ./${DSTLOC}

## git clone 
if [ ! -d ./${FILENM}/.git ]; then 
	git clone $OPTION $SRCFILE 
fi

##
## git://aaa/bbb/ccc/ddd/....//unknown.git -->
## "unknown.git" 
FILENM=${SRCFILE/${SRCFILE%/*}}

##
## "unknown"
cd ./${FILENM/.git}

## checkout back to the DATE
if [ "${SPECBR}" == "" ]; then 
	##
	## just go to sources which is close to the target target
	##
	BRANCH_NAME=`cat .git/config | grep "\[branch " | awk {'print $2'} | sed -e 's/\"//g' -e 's/]//'`
	[ "${BRANCH_NAME}" != "" ] || BRANCH_NAME="$3"
	if [ "${BRANCH_NAME}" != "" ]; then
		git checkout -q `git rev-list -n 1 --until=\"${FINAL_DATE}\" ${BRANCH_NAME}`  > /dev/null
		echo "checked out to a status until ${FINAL_DATE}"
	fi
else
	##
	## just go to sources at specific target tag 
	##
	git checkout ${SPECBR}
	echo "checked out to ${SPECBR}"
fi
