DIR=gst-python
LOC=-b 1.18 https://gitlab.freedesktop.org/gstreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREMAER_OPTS += \
	-D doc=disabled \
	-D libpython-dir=$(EXTINSTDIR)

MICB_DEPENDS = glib-2.0 python-$(PYTHON_SYSVER)-embed pygobject-3.0 \
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

