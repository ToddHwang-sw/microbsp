DIR=libinput
LOC=https://gitlab.freedesktop.org/libinput

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D documentation=false \
	-D tests=false \
	-D libwacom=false \
	-D debug-gui=false 

MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(WAYLAND_LOCATION)/lib/$(HOSTSYSTEM)-gnu/pkgconfig

MICB_DEPENDS = iconv intl glib-2.0 libffi libturbojpeg cairo-script \
	       		liblzma libxml-2.0 libudev libevdev gl mtdev

CROSS_USER_LFLAGS += -lcairo-script-interpreter
CROSS_USER_LFLAGS += -lglapi -lrt

prepare:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )

