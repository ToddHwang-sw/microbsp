DIR=gst-plugins-base
LOC=-b 1.18 https://github.com/GStreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled \
	-D base-install=$(UIXINSTDIR) 

MICB_DEPENDS = intl iconv gio-unix-2.0 gstreamer-1.0 x11 alsa \
	       graphene-1.0 pangocairo

include $(shell pwd)/../gstreamer.mk
