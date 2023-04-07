DIR=gst-plugins-base
LOC=-b 1.18 https://github.com/GStreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled \
	-D base-install=$(UIXINSTDIR)/.. 

MICB_DEPENDS = intl iconv gio-unix-2.0 x11 alsa libjpeg \
	       graphene-1.0 pango pangocairo egl            \
		   wayland-egl wayland-cursor wayland-client    \
		   gstreamer-1.0       \
		   gstreamer-base-1.0  \
		   gstreamer-net-1.0   \
		   gstreamer-check-1.0 \
		   gstreamer-controller-1.0

include $(shell pwd)/../gstreamer.mk
