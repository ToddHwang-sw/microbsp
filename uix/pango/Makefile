DIR=pango
LOC=https://github.com/GNOME

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
        -D introspection=disabled \
        -D install-tests=false 

MICB_DEPENDS = glib-2.0 gobject-2.0 gio-2.0 \
		harfbuzz-gobject libmount blkid fribidi freetype2 libdrm cairo-ft cairo

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


