DIR=cairo
LOC=https://github.com/freedesktop

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_AUTOCONF_CMD=NOCONFIGURE=0 ./autogen.sh

MICB_DEPENDS = expat gio-2.0 iconv intl json-c lzo2 gl glesv2 libdrm libpng libpng16 \
	       x11-xcb xcb-dri3 xcb-present xshmfence pixman-1 freetype2 fontconfig libudev \
	       wayland-client wayland-server blkid

MICB_CONFIGURE_OPTS += \
	--disable-gtk-doc \
	--enable-test-surfaces=no \
	--disable-full-testing \
	--enable-glesv2 \
	--enable-xml \
	--enable-xlib-xcb \
	--enable-interpreter

CROSS_USER_CFLAGS += -DCAIRO_NO_MUTEX=1
CROSS_USER_LFLAGS += -lglapi -lpthread

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


