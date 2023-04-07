
include $(TOPDIR)/scripts/env.mk

export MICB_CONFIGURE_AUTOCONF_CMD=\
		autoreconf --install -v -I$(INSTALLDIR) -I$(EXTINSTDIR)

export MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(EXTINSTDIR)/lib/pkgconfig

##
## Overriding...
##

## prefix= for meson build...
export MICB_MESON_PREFIX=$(USRNODE)

## prefix= for cmake 
export MICB_CMAKE_PREFIX=$(USRNODE)

## prefix= for cmake 
export MICB_CONFIGURE_PREFIX=$(USRNODE)

## Ninja command
export MICB_INSTALL_PLACE=$(destination)/..

## more extra configuration 
export MICB_CONFIGURE_CMD += \
		--sysconfdir=/etc \
		--localstatedir=/var
