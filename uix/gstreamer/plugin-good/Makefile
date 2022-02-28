DIR=gst-plugins-good
LOC=-b 1.18 https://github.com/GStreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled

include $(shell pwd)/../gstreamer.mk

