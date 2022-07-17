#!/bin/bash

TMPFILE=/tmp/microbsp.libs.collect

single_trace() {
	local libopt=`grep -H $1 $2 | awk '{for(i=2;i<=NF;i++) printf $i" "; print ""}'` ;

	## libdir
	local libpath=`grep -w "libdir=" $2 | sed -e "s/libdir=//g"`
	if [ "${libpath:1:1}" != "{" ]; then 
		local pref=`grep -w "prefix=" $2 | sed -e "s/prefix=//g"`
		if [[ "$pref" == "" ]]; then 
			libpath="\${prefix}"$libpath
		else
			S=${#pref}
			libpath=${libpath:$S}
			libpath="\${prefix}"$libpath
		fi
	fi
	libpath=`echo \\\\$libpath | sed -e "s/\\[$]/\\\\\[$]/g" `
	libpath=`echo $libpath | sed -e "s/\//\\\\\\\\\//g"`

	## applying prefix .... 
	local adjpath=`echo $3 | sed -e "s/\//\\\\\\\\\//g"`
	echo $libopt | \
			sed -e "s/\(\${libdir}\)/$libpath/g" | \
			sed -e "s/\(\${sharedlibdir}\)/$libpath/g" | \
			sed -e "s/\(\${prefix}\)/$adjpath/g" | \
			sed -e "s/\(\${exec_prefix}\)/$adjpath/g" | \
			sed -e "s/\#.*//g" | sed -e "s/\@.*//g"  
	
	libopt=""
	libpath=""
	adjpath=""
}

package_trace() {
	local pkg=$1
	local folder=$2
	local libreqs
	local rlib
	local path

	libreqs=`grep -H "Requires:" $pkg | awk '{for(i=2;i<=NF;i++) printf $i" "; print ""}' | sed -e "s/[,<>=]//g"` 
	libpreqs=`grep -H "Requires.private:" $pkg | awk '{for(i=2;i<=NF;i++) printf $i" "; print ""}' | sed -e "s/[,<>=]//g"` 

	if [ "$libreqs" == "" -a "$libpreqs" == "" ]; then 
		single_trace "Libs:" $pkg ${folder%/*} 
		single_trace "Libs.private:" $pkg ${folder%/*} 
		return 
	fi 

	for rlib in $libreqs $libpreqs ; do 
		if [ `cat $TMPFILE | grep $rlib | wc -l` == 0 -a \
				`echo $rlib | grep "[0_9]." | wc -l` == 0 ]; then
			echo $rlib >> $TMPFILE 
			##echo "Adding [$rlib] " >> /tmp/hello
			##echo "----------------" >> /tmp/hello
			##cat $TMPFILE >> /tmp/hello 
			##echo "----------------" >> /tmp/hello
			for path in $libspath; do 
				[ ! -f $path/pkgconfig/$rlib.pc ] || \
					package_trace $path/pkgconfig/$rlib.pc $path ;
			done
		fi 
	done 

	single_trace "Libs:" $pkg ${folder%/*} 
	single_trace "Libs.private:" $pkg ${folder%/*} 
}

[ -f $1/libs.info ] || ( \
	echo "$1/libs.info doesnt exist. \"make checkfirst\" was missed. " ; exit 0 )

libspath=`cat $1/libs.info`

[ ! -f $TMPFILE ] || \rm -rf $TMPFILE
touch $TMPFILE
package_trace $2 $3
rm $TMPFILE

