DIR=gtk
LOC=-b gtk-4.0 https://gitlab.gnome.org/GNOME

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS += \
	-D install-tests=false \
	-D build-tests=false \
	-D demos=true \
	-D introspection=disabled \
	-D man-pages=false \
	-D gtk_doc=false \
	-D print-cups=disabled \
	-D print-cloudprint=disabled \
	-D x11-backend=false \
	-D wayland-backend=true \
	-D broadway-backend=false \
	-D win32-backend=false \
	-D macos-backend=false \
	-D media-ffmpeg=disabled \
	-D media-gstreamer=disabled \
	-D base-install=$(UIXINSTDIR) 

MICB_DEPENDS = libmount bkid iconv intl glib-2.0 libffi libdrm expat libxml-2.0 libjpeg \
	pangocairo gio-unix-2.0 gdk-pixbuf-2.0 graphene-gobject-1.0 pixman-1 \
	cairo-script-interpreter 

CROSS_USER_LFLAGS += -lglapi -lrt

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )

