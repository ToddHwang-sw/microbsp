DIR=gst-plugins-ugly
LOC=-b 1.18 https://gitlab.freedesktop.org/gstreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D tests=disabled

MICB_DEPENDS = intl iconv gio-unix-2.0 \
	sndfile libdrm libxml-2.0 libcurl  \
	gstreamer-1.0       \
	gstreamer-base-1.0  \
	gstreamer-net-1.0   \
	gstreamer-check-1.0 \
	gstreamer-controller-1.0   \
	gstreamer-plugins-base-1.0 \
	gstreamer-video-1.0        \
	gstreamer-audio-1.0        \
	gstreamer-bad-audio-1.0 

include $(shell pwd)/../gstreamer.mk

