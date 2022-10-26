DIR=gst-plugins-bad
LOC=-b 1.18 https://github.com/GStreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled \
	-D base-install=$(UIXINSTDIR)

MICB_DEPENDS = intl iconv gio-unix-2.0 gstreamer-1.0 x11 alsa libdrm libxml-2.0 \
               pango pangocairo cairo gdk-pixbuf-2.0

include $(shell pwd)/../gstreamer.mk

