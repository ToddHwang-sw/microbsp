DIR=gst-plugins-ugly
LOC=-b 1.18 https://gitlab.freedesktop.org/gstreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled

include $(shell pwd)/../gstreamer.mk

