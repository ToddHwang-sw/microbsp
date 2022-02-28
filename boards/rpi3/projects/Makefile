## all folders ...
PROJDIR=$(shell ls -d */)

prepare all install uninstall:
	@for dir in $(PROJDIR); do        \
		[ ! -f $$dir/Makefile ] || make -C $$dir DESTDIR=$(destination) $@ ; \
	done

proj:
	@for dir in $(PROJDIR); do         \
		[ ! -f $$dir/Makefile ] || make -C $$dir DESTDIR=$(destination) prepare all install ; \
	done

