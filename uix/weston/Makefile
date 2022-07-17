DIR=weston
LOC=-b master https://github.com/wayland-project

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D doc=false \
	-D image-webp=false \
	-D backend-rdp=false \
	-D color-management-lcms=false \
	-D color-management-colord=false \
	-D pipewire=false \
	-D base-install=$(UIXINSTDIR)

MICB_DEPENDS = libmount bkid iconv intl pam glib-2.0 libffi libdrm expat libxml-2.0 libjpeg  \
	       libudev libevdev libsystemd dbus-1 liblz4 libcap mtdev \
	       pangocairo pixman-1 gtk4-wayland cairo-script-interpreter libinput ncurses libzstd

CROSS_USER_LFLAGS += -lglapi -lrt

prepare:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )


