#!/usr/bin/python3

import os
import sys
import re
from pkgconfig import *

#########################################
#
# G L O B A L     V A R I A B L E S 
#
##
params = []  ## parameters 
libspath = []

def getlibpath(pkg):
    for f in libspath:
        libdir = f.strip('\n')
        abspath = libdir + "/pkgconfig/" + pkg + ".pc"
        try:
            with open(abspath) as pcfile:
                pcfile.close()
                return libdir
        except:
            pass
    return ""

def package_traverse(pkgs):
    global visited
    if pkgs[0] != '':
        for node in pkgs:
            pkg = node.split()[0]
            if pkg in visited:
                pass ##  node skipped !!! 
            else:
                visited.add(pkg)
                ## print("VISIT(",pkg,")=[",visited,"]")
                ## print("VISIT(",pkg,")")
                package_trace(pkg)
    
def package_trace(pkg):
    if pkgconfig.exists(pkg):
        package_traverse( pkgconfig._query(pkg,'--print-requires').split('\n') )
        package_traverse( pkgconfig._query(pkg,'--print-requires-private').split('\n') )

        # library info ...
        print ( pkgconfig._query(pkg,'--cflags --define-prefix') )
    else:
        pass
        

if __name__ == '__main__':
   
    for i,arg in enumerate(sys.argv):
        params.append( arg )

    if len(params) < 2:
            sys.exit(0)

    ##
    ## library file ... 
    ## with open(params[1]+'/libs.info') as f:
     ##   libspath = f.readlines()
     ##   f.close()

    ##
    ## Package config ...
    os.environ['PKG_CONFIG_PATH'] = params[1] 

    ##
    ## Dynamic programming - skipping node previously visited !!
    ## 
    visited = set()
    visited.add('valgrind')

    package_trace(params[2])

    os.environ['PKG_CONFIG_PATH'] = ''
