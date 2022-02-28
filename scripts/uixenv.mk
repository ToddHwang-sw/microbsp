
include $(TOPDIR)/scripts/extenv.mk

export CROSS_USER_CFLAGS += \
	-Wno-missing-include-dirs \
        -I$(EXTINSTDIR)/usr/local/include \
        -I$(EXTINSTDIR)/usr/local/include/libdrm \
        -I$(EXTINSTDIR)/usr/local/include/libdrm/nouveau \
        -I$(EXTINSTDIR)/usr/local/include/libkms \
        -I$(EXTINSTDIR)/usr/local/include/glib-2.0 \
        -I$(EXTINSTDIR)/usr/local/include/gio-unix-2.0 \
        -I$(EXTINSTDIR)/usr/local/include/gobject-introspection-1.0 \
        -I$(EXTINSTDIR)/usr/local/lib/glib-2.0/include \
        -I$(EXTINSTDIR)/include/json-c

export CROSS_USER_CFLAGS += \
        -I$(UIXINSTDIR)/include \
        -I$(UIXINSTDIR)/include/libpng16 \
        -I$(UIXINSTDIR)/usr/local/include \
        -I$(UIXINSTDIR)/include/directfb \
        -I$(UIXINSTDIR)/include/freetype2 \
        -I$(UIXINSTDIR)/include/dbus-1.0 \
        -I$(UIXINSTDIR)/lib/dbus-1.0/include \
        -I$(UIXINSTDIR)/usr/local/include/gstreamer-1.0

export CROSS_USER_LFLAGS += \
	-L$(UIXINSTDIR)/lib  \
	-L$(UIXINSTDIR)/lib64  \
	-L$(UIXINSTDIR)/usr/local/lib \
	-L$(UIXINSTDIR)/lib/dfg-egl \

LIBS_BASIC=\
	-ldrm -lffi -lblkid -lintl -lmount -lz -llzma -lpcre -liconv -ltinfo -lxml2 -lncurses -luuid -lexpat -ljson-c 

LIBS_X11=\
	-lX11 -lX11-xcb -lXau -lXaw -lXt -lXext -lXmu -lXpm -lxcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 \
	-lxcb-present -lxcb-sync -lXfixes -lXdamage -lXxf86vm -lxshmfence -lxcb-xfixes -lxcb-xkb \
	-lwayland-client -lwayland-server -lwayland-egl \
	-lSM -lICE

LIBS_UIBASIC=\
	-ljpeg -lpng -lpng16 -lfreetype

LIBS_FFMPEG=\
	-lavcodec -lavformat -lavutil -lavfilter -lavdevice

LIBS_GLIB=\
	-lglib-2.0 -lgio-2.0 -lgobject-2.0 -lgthread-2.0 -lgmodule-2.0 

LIBS_GL=\
	-lEGL -lGL -lGLESv2 -lGLESv1_CM 

LIBS_DFB=\
	-lDFBEGL -ldirectfb -lfusion -ldirect 

LIBS_GSTREAMER=\
	-lgstbase-1.0 \
	-lgstreamer-1.0 \
	-lgstmpegts-1.0 \
	-lgstvideo-1.0 \
	-lgstapp-1.0 \
	-lgstsdp-1.0 \
	-lgstallocators-1.0 \
	-lgstrtp-1.0 \
	-lgstwebrtc-1.0 \
	-lgstgl-1.0 \
	-lgstrtsp-1.0 \
	-lgsttranscoder-1.0 \
	-lgstaudio-1.0 \
	-lgstcontroller-1.0 \
	-lgstcheck-1.0 \
	-lgstplayer-1.0 \
	-lgstnet-1.0 \
	-lgstfft-1.0 \
	-lgsttag-1.0 \
	-lgstinsertbin-1.0 \
	-lgstphotography-1.0 \
	-lgstriff-1.0 \
	-lgstpbutils-1.0 \
	-lgstbadaudio-1.0 \
	-lgstcodecparsers-1.0 \
	-lgsturidownloader-1.0 

export CROSS_USER_CFLAGS += \
	-DMB_LEN_MAX=16 \
	-D_POSIX_HOST_NAME_MAX=256 \
	-DSSIZE_MAX=2147483647 \
	-DUULONG_MAX=18446744073709551615ULL \
	-DLLONG_MAX=9223372036854775807

##
## placeholder for future reference 
##
export UILIBS=$(LIBS_BASIC) $(LIBS_GLIB)

##
## !!!! 
export WAYLAND_LOCATION = $(TOPDIR)/uix/wayland-host/build/host
export WAYLAND_BINARIES = $(WAYLAND_LOCATION)/bin

## Wayland Access 
export MICB_MESON_RUNENV += PATH=$(WAYLAND_BINARIES):$(PATH)

export MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(UIXINSTDIR)/lib/pkgconfig:$(UIXINSTDIR)/share/pkgconfig:$(UIXINSTDIR)/usr/local/lib/pkgconfig:$(UIXINSTDIR)/usr/local/share/pkgconfig

##
## Probing autogen.sh first...
export MICB_CONFIGURE_AUTOCONF_CMD = \
		autoreconf --install -v -I$(INSTALLDIR) -I$(EXTINSTDIR) -I$(UIXINSTDIR)

## Automatically inserted 
export MICB_CONFIGURE_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) \
	PATH=$(WAYLAND_BINARIES):$(PATH) \
	LIBS="$(UILIBS)"

