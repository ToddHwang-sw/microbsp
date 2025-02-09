#!/bin/bash

##
## should be called in {libs/apps/uix}/<DIR>/ 
##

LITEM=${*:$#:$#}

if   [ $# -eq 0 ]; then
	echo "ERR - arguments error "
	exit 1
else
	SRCFILE=$LITEM
	DSTFILE=${1%/*}
fi

FINAL_DATE=`cat $TOPDIR/date.inc`

GIT_IGNORE=$TOPDIR/scripts/gitignore.template

do_git_setup() {
	[ -d $DSTFILE/$1/.git ] || \rm -rf $DSTFILE/$1/.git
	[ ! -f $GIT_IGNORE ] || ( \
		[ ! -d $DSTFILE/$1 ] || cp -f $GIT_IGNORE $DSTFILE/$1/.gitignore )
	[ ! -d $DSTFILE/$1 ] || ( \
		cd $DSTFILE/$1 && \
            git init && \
            git add -A . && \
            git config user.name    "`whoami`" && \
            git config user.email   "`whoami`@someplace.com" && \
            git commit -m "Importing" . )
}

FILENM=${SRCFILE/${SRCFILE%/*}}

if [[ ${SRCFILE} == *.tar.gz ]]; then 
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	tar zxvf source$FILENM  -C $DSTFILE > /dev/null
	do_git_setup ${FILENM/.tar.gz/} 
elif [[ ${SRCFILE} == *.tar.bz2 ]]; then
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	tar jxvf source$FILENM  -C $DSTFILE > /dev/null
	do_git_setup ${FILENM/.tar.bz2/} 
elif [[ ${SRCFILE} == *.tar.xz ]]; then
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	tar xvf source$FILENM  -C $DSTFILE > /dev/null
	do_git_setup ${FILENM/.tar.xz/} 
elif [[ ${SRCFILE} == *.zip ]]; then
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	unzip source$FILENM  -d $DSTFILE 
	do_git_setup ${FILENM/.zip/} 
elif [[ ${SRCFILE} == *.git ]]; then
	[ ! -d $DSTFILE ] || mkdir -p $DSTFILE
	[ $# -lt 3 ] || SRCFILE="$2 $3 $4"
	cd $DSTFILE
	git clone --single-branch $SRCFILE 
	cd ./${FILENM/.git}
	BRANCH_NAME=`cat ./.git/config | grep "\[branch " | awk {'print $2'} | sed -e 's/\"//g' -e 's/]//'`
	[ "${BRANCH_NAME}" != "" ] || BRANCH_NAME="$3"
	## echo "Branch Name = ${BRANCH_NAME}"
	[ "${BRANCH_NAME}" == "" ] || \
			git checkout -q `git rev-list -n 1 --until=\"${FINAL_DATE}\" ${BRANCH_NAME}`
elif [[ ${SRCFILE} == *.deb ]]; then 
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	ar x --output $DSTFILE source$FILENM > /dev/null
	do_git_setup ${FILENM/.deb/}
else
	echo ""
	echo "$0  *.tar.gz,*.tar.bz2,*.zip,*.git DSTFILE"
	echo ""
fi
