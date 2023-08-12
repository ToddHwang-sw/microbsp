DIR=wireless_tools.29
LOC=https://hewlettpackard.github.io/wireless-tools

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_MAKEOPTS = \
	$(COMP_CMDS) PREFIX=$(destination)

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( \
		wget -P $(MICBSRC) $(LOC)/$(DIR).tar.gz ; \
		tar zxvf $(MICBSRC)/$(DIR).tar.gz -C $(BUILDDIR) )
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		cat ../../../$(MICB_PATCH_FN) | patch -p 1 )

prepare:

all uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); \
		PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) make $(MICB_CONFIGURE_MAKEOPTS) $@ )

install:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR) ; \
		PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) make $(MICB_CONFIGURE_MAKEOPTS) LDCONFIG="echo 1" install-bin install-dynamic )


