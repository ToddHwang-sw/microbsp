DIR=gst-plugins-base
LOC=-b 1.18 https://github.com/GStreamer

include $(TOPDIR)/scripts/uixenv.mk

GSTREAMER_OPTS += \
	-D doc=disabled \
	-D examples=disabled \
	-D tests=disabled \
	-D gl_api=opengl,gles2 \
	-D gl_platform=egl \
	-D gl_winsys=x11,wayland \
	-D xshm=disabled \
	-D base-install=$(UIXINSTDIR)/.. 

MICB_DEPENDS = intl iconv gio-unix-2.0 x11 alsa libturbojpeg \
	       graphene-1.0 pango pangocairo egl            \
		   xcb x11-xcb xshmfence xcb-shm                \
		   wayland-egl wayland-cursor wayland-client    \
		   gstreamer-1.0       \
		   gstreamer-base-1.0  \
		   gstreamer-net-1.0   \
		   gstreamer-check-1.0 \
		   gstreamer-controller-1.0

include $(shell pwd)/../gstreamer.mk
