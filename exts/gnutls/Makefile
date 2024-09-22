DIR=gnutls
LOC=-b gnutls_3_6_x https://github.com/gnutls

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_OPTS += \
	--disable-doc \
	--disable-tests \
	--disable-valgrind-tests \
	--disable-gost \
	--with-included-unistring \
	--without-nettle-mini \
	--without-included-libtasn1 \
	--without-p11-kit \
	--without-tpm \
	--without-python \
	--enable-guile \
	--disable-fuzzer-target \
	--enable-local-libopts \
	--disable-silent-rules

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_AUTOCONF_CMD = $(MICB_BOOTSTRAP_CMD)

MICB_DEPENDS = nettle hogweed libidn2 gmp libtasn1

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	$(MICB_CONFIGURE_AUTOCONF) 
	cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)


all install uninstall clean:
	[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make V=1 $(MICB_CONFIGURE_MAKEOPTS) $@ )

