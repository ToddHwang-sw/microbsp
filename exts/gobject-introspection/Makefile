DIR=gobject-introspection
LOC=-b 1.66.0 https://gitlab.gnome.org/GNOME

include $(TOPDIR)/scripts/extenv.mk

CROSS_USER_CFLAGS += \
	-I$(EXTINSTDIR)/include/python3.8 \
	-I$(EXTINSTDIR)/usr/local/include/gio-unix-2.0  \
	-I$(EXTINSTDIR)/usr/local/include/glib-2.0

CROSS_LFLAG_EXTRA += \
	-lpcre -lffi -lblkid -liconv -lintl -lmount -lz

MICB_MESON_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH)

MICB_MESON_OPTS += \
	-D gtk_doc=false	\
	-D cairo=disabled	\
	-D doctool=disabled	\
	-D build_introspection_data=false

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ]   || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )

