DIR=gobject-introspection
LOC=-b 1.66.0 https://gitlab.gnome.org/GNOME

include $(TOPDIR)/scripts/extenv.mk

MICB_MESON_OPTS += \
	-D gtk_doc=false	\
	-D cairo=disabled	\
	-D doctool=disabled	\
	-D build_introspection_data=false

MICB_DEPENDS=gio-2.0 gio-unix-2.0 python-3.8-embed iconv intl ncurses

CROSS_USER_LFLAGS += -lresolv

prepare:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )

