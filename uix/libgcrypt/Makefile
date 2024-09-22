DIR=libgcrypt-1.9.3
LOC=https://gnupg.org/ftp/gcrypt/libgcrypt/

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_OPTS += \
	--disable-doc --disable-tests --disable-rpath --disable-nls \
	--disable-large-data-tests \
	--with-libgpg-error-prefix=$(UIXINSTDIR) 

MICB_CONFIGURE_AUTOCONF_CMD = ./autogen.sh

MICB_DEPENDS = gpg-error

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.bz2 ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2; $(MICB_PATCH) )

prepare:
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


