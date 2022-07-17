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
	DSTFILE=source
fi

GIT_IGNORE=../../scripts/gitignore.template

do_git_setup() {
	[ -d $DSTFILE/$1/.git ] || \rm -rf $DSTFILE/$1/.git
	[ ! -f $GIT_IGNORE ] || cp -f $GIT_IGNORE $DSTFILE/$1/.gitignore 
	cd $DSTFILE/$1 && \
            git init && \
            git add -A . && \
            git config user.name    "`whoami`" && \
            git config user.email   "`whoami`@someplace.com" && \
            git commit -m "Importing" . 
}

FILENM=${SRCFILE/${SRCFILE%/*}}

if [[ $SRCFILE == *.tar.gz ]]; then 
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	tar zxvf source$FILENM  -C $DSTFILE > /dev/null
	do_git_setup ${FILENM/.tar.gz/} 
elif [[ $SRCFILE == *.tar.bz2 ]]; then
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	tar jxvf source$FILENM  -C $DSTFILE > /dev/null
	do_git_setup ${FILENM/.tar.bz2/} 
elif [[ $SRCFILE == *.tar.xz ]]; then
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	tar xvf source$FILENM  -C $DSTFILE > /dev/null
	do_git_setup ${FILENM/.tar.xz/} 
elif [[ $SRCFILE == *.zip ]]; then
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	unzip source$FILENM  -d $DSTFILE 
	do_git_setup ${FILENM/.zip/} 
elif [[ $SRCFILE == *.git ]]; then
	[ ! -d $DSTFILE ] || mkdir -p $DSTFILE
	[ $# -lt 3 ] || SRCFILE="$2 $3 $4"
	cd $DSTFILE ; git clone --single-branch $SRCFILE > /dev/null
elif [[ $SRCFILE == *.deb ]]; then 
	[ -f source$FILENM ] || wget -P $DSTFILE $SRCFILE
	ar x --output $DSTFILE source$FILENM > /dev/null 
	do_git_setup ${FILENM/.deb/} 
else
	echo ""
	echo "$0  <*.tar.gz,*.tar.bz2,*.zip,*.git> <DSTFILE>"
	echo ""
fi

