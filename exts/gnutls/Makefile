DIR=gnutls
LOC=-b gnutls_3_6_x https://github.com/gnutls

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_OPTS += \
	--disable-doc \
	--disable-tests \
	--disable-valgrind-tests \
	--with-included-libtasn1 \
	--with-included-unistring \
	--without-nettle-mini \
	--without-p11-kit \
	--without-tpm \
	--enable-guile \
	--enable-local-libopts \
	--disable-silent-rules

MICB_CONFIGURE_AUTOCONF_CMD = ./bootstrap

MICB_DEPENDS = nettle hogweed libidn2 gmp

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	@$(MICB_CONFIGURE_AUTOCONF) 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)


all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make V=1 $(MICB_CONFIGURE_MAKEOPTS) $@ )

