#!/bin/sh

single_trace() {
	## Cflags --
	local incopt=`grep -H Cflags: $1 | awk '{for(i=2;i<=NF;i++) printf $i" "; print ""}'`

	## includedir
	local incpath=`grep -w "includedir=" $1 | sed -e "s/includedir=//g"`
	if [ "${incpath:1:1}" != "{" ]; then 
		local pref=`grep -w "prefix=" $1 | sed -e "s/prefix=//g"`
		if [[ "$pref" == "" ]]; then 
			incpath="\${prefix}"$incpath
		else
			S=${#pref}
			incpath=${incpath:$S}
			incpath="\${prefix}"$incpath
		fi
	fi
	incpath=`echo \\\\$incpath | sed -e "s/\\[$]/\\\\\[$]/g" `
	incpath=`echo $incpath | sed -e "s/\//\\\\\\\\\//g"`

	## libdir
	local libpath=`grep -w "libdir=" $1 | sed -e "s/libdir=//g"`
	if [ "${libpath:1:1}" != "{" ]; then 
		libpath="\${prefix}"$libpath
	fi
	libpath=`echo \\\\$libpath | sed -e "s/\\[$]/\\\\\[$]/g" `
	libpath=`echo $libpath | sed -e "s/\//\\\\\\\\\//g"`

	## applying prefix .... 
	adjpath=`echo $2 | sed -e "s/\//\\\\\\\\\//g"`
	echo $incopt | sed -e "s/\(\${includedir}\)/$incpath/g" | \
			sed -e "s/\(\${libdir}\)/$libpath/g" | \
			sed -e "s/\(\${prefix}\)/$adjpath/g" | \
			sed -e "s/\(\${exec_prefix}\)/$adjpath/g"

	incopt=""
	incpath=""
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

	single_trace $1 ${2%/*} 
}

[ -f $1/libs.info ] || ( \
	echo "$1/libs.info doesnt exist. \"make checkfirst\" was missed. " ; exit 0 )

libspath=`cat $1/libs.info`

package_trace $2 $3
