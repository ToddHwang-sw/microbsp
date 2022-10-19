DIR=pango
LOC=https://gitlab.gnome.org/GNOME

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
        -D gtk_doc=false \
        -D install-tests=false 

MICB_DEPENDS = harfbuzz-gobject libmount blkid fribidi freetype2 libdrm cairo-ft

CROSS_USER_LFLAGS += -lglapi

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )


