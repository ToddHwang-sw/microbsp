DIR=dhcp
LOC=https://github.com/isc-projects

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH)

MICB_CONFIGURE_PRG = ./configure 

MICB_CONFIGURE_RUNENV += \
	LIBS="-lpthread -lssl -lcrypto -lxml2 -lz -llzma"

MICB_CONFIGURE_OPTS += \
	--with-libbind=$(EXTINSTDIR) \
	--without-randomdev \
	--enable-dhcpv6 \
	--disable-libtool

MICB_CONFIGURE_MAKEOPTS += \
	BUILD_CC=gcc \

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)
	@$(MICB_CONFIGURE_BUILDDIR_AUTOCONF) 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

