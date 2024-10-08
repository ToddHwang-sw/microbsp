DIR=linux-pam
LOC=https://github.com/linux-pam

include $(TOPDIR)/scripts/env.mk

CROSS_USER_CFLAGS += \
	-DLOGIN_NAME_MAX=256 \
	-DLINE_MAX=2048

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_AUTOCONF_CMD = ./autogen.sh 

MICB_CONFIGURE_OPTS += \
	--disable-nls \
	--disable-nis \
	--without-xauth \
	--enable-db=no \
	--disable-doc

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare: 
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )
	@[ "$@" != "install" ] || ( \
		[ -f $(destination)/etc/pam.conf ] || \
			cp -rf $(MICBSRC)/$(DIR)/conf/pam.conf $(destination)/etc/pam.conf \
		&& \
		[ -s $(destination)/include/security ] || ( \
			cd $(destination)/include ; \
			ln -s . security ) \
	)
