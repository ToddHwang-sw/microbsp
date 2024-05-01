DIR=wireless-tools
LOC=https://github.com/HewlettPackard

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_MAKEOPTS = \
	$(COMP_CMDS) PREFIX=$(destination)

WSRC=$(DIR)/wireless_tools
MAKECMD=PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) make -C ../../../$(MICBSRC)/$(WSRC) $(MICB_CONFIGURE_MAKEOPTS)

download:
	[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:

all uninstall clean:
	[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR) ; $(MAKECMD) $@ )

install:
	[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR) ; $(MAKECMD) LDCONFIG="echo 1" install-bin install-dynamic )


