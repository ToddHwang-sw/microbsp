DIR=mesa-20.0.2
MESAVER=20.0.2
LOC=https://mesa.freedesktop.org/archive/

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D wrap_mode=nodownload  \
	-D platforms=x11,wayland \
	-D build-tests=false     \
	-D llvm=false \
	-D dri=false \
	-D valgrind=false \
	-D egl=true \
	-D glx-direct=true \
	-D base-install=$(UIXINSTDIR)/

MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(WAYLAND_LOCATION)/lib/$(HOSTSYSTEM)-gnu/pkgconfig

MICB_DEPENDS = libdrm_nouveau libdrm_freedreno libdrm_vc4 zlib wayland-server libzstd

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )

