PYTHONVER=$(PYTHON_SYSVER).$(PYTHON_REV)
DIR=Python-$(PYTHONVER)
LOC=https://www.python.org/ftp/python/$(PYTHONVER)
FILENM=$(DIR).tar.xz
SAMPLEENVFILE=./usr/bin/python_env.sh
PYTHON_PKG=python$(PYTHONVER)_package.tar.bz2
PYTHONTINSTD=__install__

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	ac_cv_have_chflags=no      \
	ac_cv_buggy_getaddrinfo=no \
	ac_cv_file__dev_ptmx=yes   \
	ac_cv_file__dev_ptc=yes

MICB_CONFIGURE_OPTS += \
	--enable-optimizations        \
	--disable-profiling           \
	--with-ensurepip=install      \
	--without-system-libmpdec     \
	--without-decimal-contextvar

MICB_CONFIGURE_PRG = ./configure

MICB_DEPENDS = expat openssl ncurses ncursesw libffi sqlite3 uuid

## System Library ...
SYS_X86_LIBDIR=/lib/x86_64-linux-gnu

download:
	@[ -d $(BUILDDIR)/$(DIR)   ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(FILENM) ] || ( $(UNCOMPRESS) $(LOC)/$(FILENM) ; $(MICB_PATCH) )

prepare:
	@[ "$(_ARCH_)" != "x86_64-vm" ] || ( \
		[ -f $(SYS_X86_LIBDIR)/libtinfow.so.6 ] || ( \
			[ ! -f $(SYS_X86_LIBDIR)/libtic.so.6 ] || ( \
				echo ""                        ; \
				echo "Linking libtinfow.so.6 " ; \
				echo ""                        ; \
				sudo ln -s $(SYS_X86_LIBDIR)/libtic.so.6 $(SYS_X86_LIBDIR)/libtinfow.so.6  ) ) )
	@$(MICB_DUP_SOURCES)
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

