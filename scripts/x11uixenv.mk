
include $(TOPDIR)/scripts/uixenv.mk

export X11INSTDIR=$(UIXINSTDIR)/usr/local

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

export MICB_CONFIGURE_MAKEOPTS=\
		V=1 DESTDIR=$(X11INSTDIR)


## Just simply 
export CROSS_USER_CFLAGS += -I$(X11INSTDIR)/include 
