
include $(TOPDIR)/scripts/uixenv.mk

export X11INSTDIR=$(UIXINSTDIR)/local

export MICB_CONFIGURE_OPTS += \
	--disable-specs \
	--without-xmlto \
	--without-fop \
	--without-xsltproc  \
	--enable-malloc0returnsnull=yes \
	--enable-shared \
	--enable-static

##
##
##  C O N F I G U R E 
##
##
export MICB_CONFIGURE_AUTOCONF_CMD=\
		autoreconf --install -v -I$(INSTALLDIR) -I$(EXTINSTDIR) -I$(X11INSTDIR)/share -I$(X11INSTDIR)/share/aclocal ; \

export MICB_CONFIGURE_PRG=../../../$(MICBSRC)/$(DIR)/configure

## prefix= for meson build...
export MICB_MESON_PREFIX=

## prefix= for cmake 
export MICB_CMAKE_PREFIX=

## prefix= for cmake 
export MICB_CONFIGURE_PREFIX=

## option..
export MICB_CONFIGURE_MAKEOPTS= V=1 DESTDIR=$(X11INSTDIR)


## Just simply 
export CROSS_USER_CFLAGS += -I$(X11INSTDIR)/include 
