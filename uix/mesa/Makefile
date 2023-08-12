DIR=mesa-21.2.2
LOC=https://mesa.freedesktop.org/archive/

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D wrap_mode=nodownload    \
	-D platforms=x11,wayland   \
	-D build-tests=false       \
	-D llvm=disabled           \
	-D libunwind=disabled      \
	-D valgrind=disabled       \
	-D egl=enabled             \
	-D gles1=enabled           \
	-D gles2=enabled           \
	-D opengl=true             \
	-D glx=disabled            \
	-D dri3=enabled            \
	-D vulkan-drivers=broadcom      \
	-D gallium-drivers=vc4,v3d,kmsro\
	-D glx-direct=true         \
	-D base-install=$(UIXINSTDIR)/..


MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(WAYLAND_LOCATION)/lib/$(HOSTSYSTEM)-gnu/pkgconfig

MICB_DEPENDS = libdrm libzstd expat zlib \
		x11 x11-xcb xcb xcb-glx xcb-present xcb-randr xcb-dri2 xcb-dri3 xcb-shm xcb-xf86dri xcb-xfixies \
		xext xextproto xshmfence xrandr \
		wayland-egl wayland-client wayland-protocols wayland-scanner wayland-server

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )

