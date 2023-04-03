DIR=gstreamer-vaapi
LOC=-b 1.18 https://gitlab.freedesktop.org/gstreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled \
	-D with_glx=no    \
	-D base-install=$(UIXINSTDIR)/..

MICB_DEPENDS = glib-2.0 libva libva-wayland libva-drm libva-x11 libdrm libudev x11 egl xrandr \
	wayland-egl wayland-client wayland-cursor \
	gstreamer-1.0       \
	gstreamer-base-1.0  \
	gstreamer-net-1.0   \
	gstreamer-check-1.0 \
	gstreamer-controller-1.0   \
	gstreamer-plugins-base-1.0 \
	gstreamer-video-1.0        \
	gstreamer-audio-1.0        \
	gstreamer-gl-wayland-1.0   \
	gstreamer-codecparsers-1.0 \
	gstreamer-bad-audio-1.0 

include $(shell pwd)/../gstreamer.mk
