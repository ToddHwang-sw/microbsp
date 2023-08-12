DIR=util-linux
LOC=-b stable/v2.37 https://github.com/util-linux

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_OPTS +=\
	--enable-largefile  \
	--enable-libuuid \
	--enable-libuuid-force-uuidd \
	--enable-libblkid \
	--enable-blkid \
	--enable-libmount \
	--enable-mount \
	--enable-mountpoint \
	--enable-widechar \
	--disable-all-programs \
	--with-ncursesw \
	--with-readline \
	--with-tinfo \
	--without-utempter \
	--without-libiconv-prefix \
	--disable-makeinstall-chown

MICB_CONFIGURE_AUTOCONF_CMD=./autogen.sh

MICB_CONFIGURE_RUNENV +=\
		ac_cv_func_unshare=yes ac_cv_func_setns=yes \
		LIBS="-lrt" 

MICB_DEPENDS = ncurses

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare: 
	@$(MICB_CONFIGURE_AUTOCONF) 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

