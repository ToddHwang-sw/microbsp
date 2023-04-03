DIR=mesa-20.0.2
MESAVER=20.0.2
LOC=https://mesa.freedesktop.org/archive/

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D wrap_mode=nodownload   \
	-D glx=disabled           \
	-D platforms=x11,drm,wayland  \
	-D build-tests=false      \
	-D llvm=false             \
	-D libunwind=false        \
	-D valgrind=false         \
	-D egl=true               \
	-D vulkan-drivers=''      \
	-D dri-drivers=''         \
	-D gallium-drivers=vc4,v3d,kmsro  \
	-D glx-direct=true        \
	-D base-install=$(UIXINSTDIR)/..


MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(WAYLAND_LOCATION)/lib/$(HOSTSYSTEM)-gnu/pkgconfig


MICB_DEPENDS = libdrm_nouveau libdrm_freedreno libdrm_vc4 zlib wayland-server libzstd expat \
		x11 x11-xcb xcb xcb-glx xcb-present xcb-randr xcb-dri2 xcb-dri3 xcb-shm xcb-xf86dri xcb-xfixies \
		xext xextproto xshmfence xrandr \
		wayland-egl wayland-client wayland-protocols 


prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )

