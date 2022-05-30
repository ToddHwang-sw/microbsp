DIR=Python-3.8.6
PYTHONVER=3.8.6
LOC=https://www.python.org/ftp/python/$(PYTHONVER)
FILENM=$(DIR).tar.xz
SAMPLEENVFILE=bin/python_env.sh
PYTHON_PKG=python$(PYTHONVER)_package.tar.bz2
PYTHONTINSTD=__install__
PYTHON_CMDS=$(CROSS_COMP_CMDS) READELF="$(CROSS_COMP_PREFIX)readelf"

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -f $(MICBSRC)/$(FILENM) ] || wget -P $(MICBSRC) $(LOC)/$(FILENM)
	@tar xvf $(MICBSRC)/$(FILENM) -C $(BUILDDIR); \
		cd $(BUILDDIR)/$(DIR); \
		ac_cv_have_chflags=no \
		ac_cv_buggy_getaddrinfo=no \
		ac_cv_file__dev_ptmx=yes \
		ac_cv_file__dev_ptc=yes \
		PKG_CONFIG_PATH=$(INSTALLDIR)/lib/pkgconfig \
			$(PYTHON_CMDS) ./configure --prefix= --target=$(PLATFORM) --build=i686-linux --host=$(PLATFORM) \
				--disable-static --enable-shared \
				--enable-optimizations --disable-profiling \
				--with-ensurepip

all: 
	@cd $(BUILDDIR)/$(DIR); \
		make build_all; \
		[ ! -d $(PYTHONTINSTD) ] || \rm -rf $(PYTHONTINSTD) ; \
		make DESTDIR=`pwd`/$(PYTHONTINSTD) install; \
		cd $(PYTHONTINSTD); \
			printf "#!/bin/sh\n"				>  $(SAMPLEENVFILE)	;	\
			printf "\n"					>> $(SAMPLEENVFILE)	;	\
			printf "BASE=\$$1\n"				>> $(SAMPLEENVFILE)	;	\
			printf "\n"					>> $(SAMPLEENVFILE)	;	\
			printf "export PYTHONHOME=\$$BASE\n"		>> $(SAMPLEENVFILE)	;	\
			printf "export PYTHONPATH=\$$BASE/bin\n"	>> $(SAMPLEENVFILE)	;	\
			printf "export PATH=\$$PATH:\$$PYTHONPATH\n"	>> $(SAMPLEENVFILE)	;	\
			printf "export LD_LIBRARY_PATH=/lib:/usr/lib:\$$PYTHONHOME/lib\n"	>> $(SAMPLEENVFILE)	;	\
			chmod +x $(SAMPLEENVFILE)
	@cd $(BUILDDIR)/$(DIR)/$(PYTHONTINSTD); \
		find bin/          -name "python3.6*" -exec $(CROSS_COMP_PREFIX)strip {} \;  ;\
		find lib/python3.6 -name "*.so*"     -exec $(CROSS_COMP_PREFIX)strip {} \;  ;\
		tar jcvf $(PYTHON_PKG) bin/ include/ lib/ usr/

install:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		[ -d $(destination) ] || mkdir -p $(destination);  \
		cd $(BUILDDIR)/$(DIR)/$(PYTHONTINSTD);             \
		tar jxvf $(PYTHON_PKG) -C $(destination);          \
	)

clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		make clean ; \
		[ ! -d $(PYTHONTINSTD) ] || \rm -rf $(PYTHONTINSTD); \
	)

uninstall:

