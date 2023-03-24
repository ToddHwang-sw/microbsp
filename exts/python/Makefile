PYTHONVER=3.10.8
DIR=Python-$(PYTHONVER)
LOC=https://www.python.org/ftp/python/$(PYTHONVER)
FILENM=$(DIR).tar.xz
SAMPLEENVFILE=./usr/bin/python_env.sh
PYTHON_PKG=python$(PYTHONVER)_package.tar.bz2
PYTHONTINSTD=__install__

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	ac_cv_have_chflags=no \
	ac_cv_buggy_getaddrinfo=no \
	ac_cv_file__dev_ptmx=yes \
	ac_cv_file__dev_ptc=yes

MICB_CONFIGURE_OPTS += \
	--enable-optimizations \
	--disable-profiling \
	--with-openssl=$(INSTALLDIR) \
	--with-ensurepip=install

MICB_CONFIGURE_PRG = ./configure

MICB_DEPENDS = expat openssl ncurses libffi

CROSS_USER_CFLAGS += -I$(EXTINSTDIR)/include 
CROSS_USER_LFLAGS += -L$(EXTINSTDIR)/lib -lgcov 

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(FILENM) ] || \
		wget -P $(MICBSRC) $(LOC)/$(FILENM)
	@tar xvf $(MICBSRC)/$(FILENM) -C $(BUILDDIR) > /dev/null 
	@[ ! -f $(MICB_PATCH_FN) ] || \
		(cd $(BUILDDIR)/$(DIR) && cat ../../../$(MICB_PATCH_FN) | patch -p 1)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all: 
	@cd $(BUILDDIR)/$(DIR); \
		make build_all; \
		[ ! -d $(PYTHONTINSTD) ] || \rm -rf $(PYTHONTINSTD) ; \
		make DESTDIR=`pwd`/$(PYTHONTINSTD) install; \
		cd $(PYTHONTINSTD); \
			printf "#!/bin/sh\n"                    >  $(SAMPLEENVFILE)	;	\
			printf "\n"                             >> $(SAMPLEENVFILE)	;	\
			printf "BASE=\$$1\n"                    >> $(SAMPLEENVFILE)	;	\
			printf "\n"                             >> $(SAMPLEENVFILE)	;	\
			printf "export PYTHONHOME=\$$BASE\n"    >> $(SAMPLEENVFILE)	;	\
			printf "export PYTHONPATH=\$$BASE/bin\n">> $(SAMPLEENVFILE)	;	\
			printf "export PATH=\$$PATH:\$$PYTHONPATH\n"                             >> $(SAMPLEENVFILE)	;	\
			printf "export LD_LIBRARY_PATH=\$$LD_LIBRARY_PATH:\$$PYTHONHOME/lib\n"   >> $(SAMPLEENVFILE)	;	\
			chmod +x $(SAMPLEENVFILE)
	@cd $(BUILDDIR)/$(DIR)/$(PYTHONTINSTD); \
		tar jcvf $(PYTHON_PKG) * 

install:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		[ -d $(destination) ] || mkdir -p $(destination);  \
		cd $(BUILDDIR)/$(DIR)/$(PYTHONTINSTD);             \
		tar jxvf $(PYTHON_PKG) -C $(MICB_INSTALL_PLACE);   \
	)

clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		make clean ; \
		[ ! -d $(PYTHONTINSTD) ] || \rm -rf $(PYTHONTINSTD); \
	)

uninstall:

