DIR=gstreamer
LOC=-b 1.18 https://gitlab.freedesktop.org/gstreamer/

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D gtk_doc=disabled \
	-D doc=disabled\
	-D orc=disabled\
	-D examples=disabled\
	-D tests=disabled \
	-D benchmarks=disabled

include $(shell pwd)/../gstreamer.mk

