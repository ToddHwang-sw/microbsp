DIR=wpa_supplicant-2.9
LOC=https://w1.fi/releases

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_MAKEOPTS += \
	$(COMP_CMDS) V=1 LIBS="$(DEPLOYED_LIBFLAGS)"

MICB_DEPENDS = openssl libnl-cli-3.0 ncurses

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || wget -P $(MICBSRC) $(LOC)/$(DIR).tar.gz
	@[ -d $(BUILDDIR)/$(DIR) ] || tar zxvf $(MICBSRC)/$(DIR).tar.gz -C $(BUILDDIR)
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		cat ../../../$(MICB_PATCH_FN) | patch -p 1; \
		cp  -rf ../../../patch/defconfig wpa_supplicant/.config )

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR)/wpa_supplicant ; \
		PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) make $(MICB_CONFIGURE_MAKEOPTS) $@ )

