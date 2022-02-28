DIR=glibc

include $(TOPDIR)/arch/$(_ARCH_)/env.mk

CROSS_USER_CFLAGS=
CROSS_USER_LFLAGS=

GLIBCDIR=$(TOPDIR)/gnu/glibc/$(GLIBC)

##
## LOCALE installation flag : It will take very long time. 
##
LOCALE_FLAG=0

GLIBC_TARGET_FILE=$(GLIBC)-$(PLATFORM).tar.gz

prepare:
	@[ -d $(BUILDDIR)/$(GLIBC) ] || mkdir -p $(BUILDDIR)/$(GLIBC)
	@[ -d $(GLIBCDIR) ] || ( echo "$(GLIBC) has not been compiled..."; exit 1 )
	@[ -f $(BUILDDIR)/$(GLIBC_TARGET_FILE) ] || ( \
		cd $(BUILDDIR)/$(GLIBC); \
		$(COMP_CMDS) $(GLIBCDIR)/configure --prefix= \
			--build=i686-linux --host=$(PLATFORM) --target=$(PLATFORM)  \
			--disable-multi-arch )

all clean:
	@[ -f $(BUILDDIR)/$(GLIBC_TARGET_FILE) ] || ( \
		[ ! -d $(BUILDDIR)/$(GLIBC) ] || ( cd $(BUILDDIR)/$(GLIBC); make -j8 $@ ))

##
## board-specific epilogue processing...
## 
install:
	@[ -f $(BUILDDIR)/$(GLIBC_TARGET_FILE) ] || ( \
		[ ! -d $(BUILDDIR)/$(GLIBC) ] || ( \
			cd $(BUILDDIR)/$(GLIBC); \
			make install_root=`pwd`/../target install;  \
			[ "$(LOCALE_FLAG)" = "0" ] || make install_root=`pwd`/../target localedata/install-locales; \
			cd ../target; \
			tar zcvf ../$(GLIBC_TARGET_FILE) * > /dev/null ) \
	)

##
## CAUTION !! - This should happen at the end of ISO image build. 
##
install_glibc:
	@[ ! -f $(BUILDDIR)/$(GLIBC_TARGET_FILE) ] || \
			tar zxvf $(BUILDDIR)/$(GLIBC_TARGET_FILE) -C $(destination) > /dev/null
	@[ "$(_CORE_)" != "x86_64" ] || (                                   \
		[ -d $(destination)/lib64 ] || mkdir -p $(destination)/lib64  ; \
		[ -f $(destination)/lib64/ld-linux-x86-64.so.2 ] || (           \
			cd $(destination)/lib64;                               \
			ln -s ../lib/$(subst glibc,ld,$(GLIBC)).so ld-linux-x86-64.so.2 \
		))

