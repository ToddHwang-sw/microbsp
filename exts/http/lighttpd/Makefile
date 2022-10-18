DIR=lighttpd1.4
LOC=https://github.com/lighttpd

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_OPTS += \
	--without-lua \
	--without-libunwind \
	--with-openssl \
	--with-gnutls \
	--with-libev \
	--with-zstd \
	--with-bzip2 \
	--with-nettle 

MICB_DEPENDS = iconv intl glib-2.0 openssl gnutls pam libzstd nettle 

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( \
		$(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH); \
		mkdir -p $(MICBSRC)/$(DIR)/m4 )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

