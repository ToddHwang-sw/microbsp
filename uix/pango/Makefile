PANGOVER=1.50
DIR=pango-$(PANGOVER).11
LOC=https://download.gnome.org/sources/pango/$(PANGOVER)

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
        -D introspection=disabled \
        -D install-tests=false 

MICB_DEPENDS = harfbuzz-gobject libmount blkid fribidi freetype2 libdrm cairo-ft cairo

CROSS_USER_LFLAGS += -lglapi

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )


