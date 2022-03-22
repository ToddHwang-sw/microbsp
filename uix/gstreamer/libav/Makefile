DIR=gst-libav
LOC=-b 1.10 https://gitlab.freedesktop.org/gstreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled

MICB_DEPENDS = intl iconv alsa gstreamer-1.0 glib-2.0 libavdevice

include $(shell pwd)/../gstreamer.mk

