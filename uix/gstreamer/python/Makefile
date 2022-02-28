DIR=gst-python
LOC=-b 1.18 https://gitlab.freedesktop.org/gstreamer

include $(TOPDIR)/scripts/uixenv.mk

CROSS_USER_CFLAGS += \
	-I$(EXTINSTDIR)/include/python3.8  \
	-I$(UIXINSTDIR)/usr/local/include/pygobject-3.0/

GSTREMAER_OPTS+=\
	-D doc=disabled \
	-D libpython-dir=$(EXTINSTDIR)/lib \
	-D pygi-overrides-dir=$(UIXINSTDIR)/usr/local/lib/python3.8/site-packages

include $(shell pwd)/../gstreamer.mk

