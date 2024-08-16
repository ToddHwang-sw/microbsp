DIR=wpa_supplicant-2.9
LOC=https://w1.fi/releases

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_MAKEOPTS += \
	$(COMP_CMDS) V=1 LIBS="$(DEPLOYED_LIBFLAGS)"

MICB_DEPENDS = openssl libnl-cli-3.0 ncurses

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz ; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)
	@[ -f $(BUILDDIR)/$(DIR)/wpa_supplicant/.config ] || \
		cp patch/defconfig $(BUILDDIR)/$(DIR)/wpa_supplicant/.config 

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR)/wpa_supplicant ; \
		PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) make $(MICB_CONFIGURE_MAKEOPTS) $@ )

