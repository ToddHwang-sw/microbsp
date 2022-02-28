DIR=gst-plugins-base
LOC=-b 1.18 https://github.com/GStreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled \
	-D base-install=$(UIXINSTDIR) 

include $(shell pwd)/../gstreamer.mk
