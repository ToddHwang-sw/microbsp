
include $(TOPDIR)/scripts/env.mk

export MICB_CONFIGURE_AUTOCONF_CMD=\
		autoreconf --install -v -I$(INSTALLDIR) -I$(EXTINSTDIR)

export MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(EXTINSTDIR)/lib/pkgconfig:$(EXTINSTDIR)/share/pkgconfig:$(EXTINSTDIR)/usr/local/lib/pkgconfig:$(EXTINSTDIR)/usr/local/share/pkgconfig

##
## GNU S/W bootstrap process takes very long time, so we use this file to prevent multiple bootstrapping by uncareful configuration challenges..
export BSDONE_FILE = ./__bootstrap.done

