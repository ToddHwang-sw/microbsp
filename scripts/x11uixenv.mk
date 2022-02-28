
include $(TOPDIR)/scripts/uixenv.mk

export X11INSTDIR=$(UIXINSTDIR)/usr/local

##
## Probing autogen.sh first...
export MICB_CONFIGURE_AUTOCONF_CMD = \
		autoreconf --install -v -I$(INSTALLDIR) -I$(EXTINSTDIR) -I$(X11INSTDIR)/share -I$(X11INSTDIR)/share/aclocal ; \

##
## Update Destination 
export MICB_CONFIGURE_MAKEOPTS += V=1
export MICB_CONFIGURE_MAKEOPTS := $(filter-out DESTDIR=$(destination),$(MICB_CONFIGURE_MAKEOPTS))
export MICB_CONFIGURE_MAKEOPTS += DESTDIR=$(X11INSTDIR)

export MICB_CONFIGURE_OPTS += \
	--disable-specs \
	--without-xmlto \
	--without-fop \
	--without-xsltproc  \
        --enable-malloc0returnsnull=yes

