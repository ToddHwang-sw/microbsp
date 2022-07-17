DIR=gstreamer
LOC=-b 1.18 https://gitlab.freedesktop.org/gstreamer/

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D gtk_doc=disabled \
	-D doc=disabled\
	-D examples=disabled\
	-D tests=disabled \
	-D benchmarks=disabled

MICB_DEPENDS = intl iconv gio-unix-2.0

include $(shell pwd)/../gstreamer.mk

