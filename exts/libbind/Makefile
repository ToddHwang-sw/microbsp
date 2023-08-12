DIR=bind-9.11.36
LOC=https://downloads.isc.org/isc/bind9/9.11.36

include $(TOPDIR)/scripts/extenv.mk

ifeq ($(_CORE_),aarch64)
        MICB_CONFIGURE_PLATFORM=arm-linux
endif

MICB_CONFIGURE_RUNENV += \
	BUILD_CC=gcc

MICB_CONFIGURE_OPTS += \
	--without-python  \
	--with-randomdev=no \
	--with-ecdsa=no  \
	--with-gost=no   \
	--with-eddsa=no \
	--disable-linux-caps 

MICB_DEPENDS=openssl libxml-2.0 ncurses

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )
