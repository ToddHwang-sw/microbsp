#!/bin/sh

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
	local libreqs
	local rlib
	local path

	libreqs=`grep -H "Requires:" $1 | awk '{for(i=2;i<=NF;i++) printf $i" "; print ""}' | sed -e "s/[,<>=]//g"` 
	libpreqs=`grep -H "Requires.private:" $1 | awk '{for(i=2;i<=NF;i++) printf $i" "; print ""}' | sed -e "s/[,<>=]//g"` 

	for rlib in $libreqs $libpreqs ; do 
		for path in $libspath; do 
			[ ! -f $path/pkgconfig/$rlib.pc ] || \
				package_trace $path/pkgconfig/$rlib.pc $path ;
		done ;
	done 

	single_trace "Libs:" $1 ${2%/*} 
	single_trace "Libs.private:" $1 ${2%/*} 
}

[ -f $1/libs.info ] || ( \
	echo "$1/libs.info doesnt exist. \"make checkfirst\" was missed. " ; exit 0 )

libspath=`cat $1/libs.info`

package_trace $2 $3
