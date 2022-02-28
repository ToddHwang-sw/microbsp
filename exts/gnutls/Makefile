DIR=gnutls
LOC=https://github.com/gnutls

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_AUTOCONF_CMD = \
	./bootstrap 

MICB_CONFIGURE_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) \

MICB_CONFIGURE_OPTS += \
	--disable-doc \
	--disable-tests \
	--disable-valgrind-tests \
	--with-included-libtasn1 \
	--with-included-unistring \
	--without-p11-kit \
	--without-tpm \
	--disable-silent-rules

MICB_CONFIGURE_MAKEOPTS += \
	LIBS="-lnettle -lhogweed -lgmp"

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ -f $(BUILDDIR)/$(DIR)/$(BSDONE_FILE) ] || ( \
		$(MICB_CONFIGURE_AUTOCONF) ; \
		touch $(BUILDDIR)/$(DIR)/$(BSDONE_FILE) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make V=1 $(MICB_CONFIGURE_MAKEOPTS) $@ )

