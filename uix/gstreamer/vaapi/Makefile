DIR=gstreamer-vaapi
LOC=-b 1.18 https://gitlab.freedesktop.org/gstreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled

MICB_DEPENDS = gstreamer-1.0 glib-2.0 libva

include $(shell pwd)/../gstreamer.mk
