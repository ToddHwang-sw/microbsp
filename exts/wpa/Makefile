DIR=wpa_supplicant-2.9
LOC=https://w1.fi/releases

include $(TOPDIR)/scripts/extenv.mk

CROSS_USER_CFLAGS += \
	-I$(INSTALLDIR)/include/libnl3

MICB_CONFIGURE_MAKEOPTS += \
	$(COMP_CMDS) V=1 LIBS="-ltinfo -lnl-3 -lnl-genl-3 -lssl -lcrypto -ldl"

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -f source/$(DIR).tar.gz ] || wget -P source $(LOC)/$(DIR).tar.gz
	@[ -d $(BUILDDIR)/$(DIR) ] || tar zxvf source/$(DIR).tar.gz -C $(BUILDDIR)
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		cat ../../../patch/patch | patch -p 1; \
		cp  -rf ../../../patch/defconfig wpa_supplicant/.config )

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR)/wpa_supplicant ; \
		PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) make $(MICB_CONFIGURE_MAKEOPTS) $@ )

