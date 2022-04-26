DIR=libinput
LOC=https://github.com/wayland-project

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D documentation=false \
	-D tests=false \
	-D libwacom=false \
	-D debug-gui=false 

MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(WAYLAND_LOCATION)/lib/$(HOSTSYSTEM)-gnu/pkgconfig

MICB_DEPENDS = iconv intl glib-2.0 libffi libjpeg cairo-script-interpreter \
	       		liblzma libxml-2.0 libudev libevdev

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

