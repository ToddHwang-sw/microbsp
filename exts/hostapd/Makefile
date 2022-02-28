DIR=hostapd-2.9
LOC=https://w1.fi/releases

include $(TOPDIR)/scripts/extenv.mk

CROSS_USER_CFLAGS += -I$(INSTALLDIR)/include/libnl3

MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS) V=1

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES) 
	@[ -f $(BUILDDIR)/$(DIR)/hostapd/.config ] || \
		cp patch/config $(BUILDDIR)/$(DIR)/hostapd/.config 

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR)/hostapd; \
			PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) make $(MICB_CONFIGURE_MAKEOPTS) $@ )


