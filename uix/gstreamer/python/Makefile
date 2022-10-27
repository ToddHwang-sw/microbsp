DIR=gst-python
LOC=-b 1.18 https://gitlab.freedesktop.org/gstreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREMAER_OPTS += \
	-D doc=disabled \
	-D libpython-dir=$(EXTINSTDIR)

MICB_DEPENDS = gstreamer-1.0 glib-2.0 python-3.10-embed pygobject-3.0

include $(shell pwd)/../gstreamer.mk

