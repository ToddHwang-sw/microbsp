DIR=gdk-pixbuf
LOC=https://gitlab.gnome.org/GNOME

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D gtk_doc=false \
	-D docs=false \
	-D man=false \
	-D installed_tests=false \
	-D introspection=disabled 

MICB_DEPENDS = libpng libpng16 libjpeg iconv intl mount blkid gobject-2.0

CROSS_USER_LFLAGS += -lm

prepare:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )


