DIR=cairo
LOC=https://gitlab.freedesktop.org/cairo

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D tests=disabled             \
	-D gtk_doc=false              \
	-D dwrite=disabled            \
	-D xlib=disabled              \
	-D gtk2-utils=disabled        \
	-D spectre=disabled

CROSS_USER_CFLAGS += -DCAIRO_NO_MUTEX=1 -DHAVE_CTIME_R

MICB_DEPENDS = fontconfig freetype2 libpng libpng16 xcb zlib pixman-1 xcb-shm xcb-render glib-2.0 gobject-2.0 lzo2

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )

