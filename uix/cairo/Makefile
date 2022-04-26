DIR=cairo
LOC=https://github.com/freedesktop

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_AUTOCONF_CMD=NOCONFIGURE=0 ./autogen.sh

MICB_DEPENDS = expat gio-2.0 iconv intl json-c lzo2 gl glesv2 libdrm libpng libpng16 \
	       x11-xcb xcb-dri3 xcb-present xshmfence pixman-1 freetype fontconfig libudev \
	       wayland-client wayland-server 

MICB_CONFIGURE_OPTS += \
	--disable-gtk-doc \
	--enable-test-surfaces=no \
	--disable-full-testing \
	--enable-glesv2 \
	--enable-xml \
	--enable-xlib-xcb

CROSS_USER_CFLAGS += \
	-lglapi \
	-DCAIRO_NO_MUTEX=1

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


