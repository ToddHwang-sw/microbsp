DIR=gst-plugins-good
LOC=-b 1.18 https://github.com/GStreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled

MICB_DEPENDS = intl iconv gio-unix-2.0 x11 alsa \
		   cairo cairo-gobject gdk-pixbuf-2.0 \
	       pango pangocairo    \
		   xfixes xdamage      \
		   gstreamer-1.0       \
		   gstreamer-base-1.0  \
		   gstreamer-net-1.0   \
		   gstreamer-check-1.0 \
		   gstreamer-controller-1.0   \
		   gstreamer-plugins-base-1.0 \
		   gstreamer-video-1.0

include $(shell pwd)/../gstreamer.mk

