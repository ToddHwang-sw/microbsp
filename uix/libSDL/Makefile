DIR=SDL
LOC=-b 2.0.22 https://github.com/libsdl-org

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_OPTS += \
	--enable-arm-neon \
	--disable-loadso \
	--disable-joystick \
	--disable-haptic

MICB_CONFIGURE_MAKEOPTS += \
	WAYLAND_SCANNER=$(WAYLAND_BINARIES)/wayland-scanner	

MICB_DEPENDS = libdrm_nouveau libdrm_freedreno libdrm_vc4 zlib wayland-client \
	       wayland-server \
	       glesv2 egl libudev expat xcb-xfixes xshmfence intl uuid blkid  \
	       xcb-dri3 xcb-present xcb-sync ncuerses 

CROSS_USER_LFLAGS += -lglapi

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

