
## all folders ...
PROJDIR=$(shell ls -d */)

prepare all install:
	@for dir in $(PROJDIR); do         \
		[ ! -f $$dir/Makefile ] || make -C $$dir DESTDIR=$(destination) $@ ; \
	done

