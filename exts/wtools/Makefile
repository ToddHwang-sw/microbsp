DIR=wireless_tools.29
LOC=https://hewlettpackard.github.io/wireless-tools

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_MAKEOPTS = \
	$(COMP_CMDS) PREFIX=$(destination)

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -f source/$(DIR).tar.gz ] || wget -P source $(LOC)/$(DIR).tar.gz
	@[ -d $(BUILDDIR)/$(DIR) ] || tar zxvf source/$(DIR).tar.gz -C $(BUILDDIR)
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		cat ../../../patch/patch | patch -p 1 )

all uninstall uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); \
		PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) make $(MICB_CONFIGURE_MAKEOPTS) $@ )

install:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR) ; \
		PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) make $(MICB_CONFIGURE_MAKEOPTS) LDCONFIG="echo 1" install-bin install-dynamic )


