
include $(TOPDIR)/scripts/extenv.mk

export CROSS_USER_CFLAGS += \
	-DMB_LEN_MAX=16 \
	-D_POSIX_HOST_NAME_MAX=256 \
	-DSSIZE_MAX=2147483647 \
	-DUULONG_MAX=18446744073709551615ULL \
	-DLLONG_MAX=9223372036854775807

## Wayland host tool 
export WAYLAND_LOCATION = $(TOPDIR)/uix/wayland-host/build/host
export WAYLAND_BINARIES = $(WAYLAND_LOCATION)/bin

## Wayland host tool included 
export MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(UIXINSTDIR)/lib/pkgconfig:$(UIXINSTDIR)/share/pkgconfig:$(UIXINSTDIR)/local/lib/pkgconfig:$(UIXINSTDIR)/local/share/pkgconfig:$(WAYLAND_LOCATION)/lib/$(HOSTSYSTEM)-gnu/pkgconfig

##
## Probing autogen.sh first...
export MICB_CONFIGURE_AUTOCONF_CMD = \
	autoreconf --install -v -I$(INSTALLDIR) -I$(EXTINSTDIR) -I$(UIXINSTDIR)

## Wayland Access - MESON
export MICB_MESON_RUNENV += \
	PATH=$(WAYLAND_BINARIES):$(PATH)

## Wayland Access - CONFIGURE
export MICB_CONFIGURE_RUNENV += \
	PATH=$(WAYLAND_BINARIES):$(PATH)

