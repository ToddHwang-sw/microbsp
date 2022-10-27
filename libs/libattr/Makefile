DIR=attr-2.5.1
LOC=https://download.savannah.gnu.org/releases/attr

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_OPTS += \
		--enable-largefile \
		--with-libiconv-prefix=$(INSTALLDIR) \
		--with-libintl-prefix=$(INSTALLDIR) \

MICB_DEPENDS = iconv intl 

prepare: 
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


