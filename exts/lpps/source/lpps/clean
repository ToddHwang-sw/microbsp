#!/bin/sh

[ ! -f Makefile ] || make distclean 
find . -name ".deps" -exec \rm -rf {} \;
\rm -rf \
	aclocal* \
	auto* \
	compile \
	configure \
	depcomp \
	install* \
	Makefile.in \
	missing 
