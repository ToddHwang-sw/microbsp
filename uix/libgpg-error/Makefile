DIR=libgpg-error-1.43
LOC=https://gnupg.org/ftp/gcrypt/libgpg-error/

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_OPTS += \
	--disable-doc --disable-tests

MICB_CONFIGURE_AUTOCONF_CMD = ./autogen.sh

MICB_CONFIGURE_MAKEOPTS += V=1 

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.bz2 ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


