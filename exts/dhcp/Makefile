DIR=dhcp
LOC=https://github.com/isc-projects

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_PRG = ./configure 

MICB_CONFIGURE_OPTS += \
	--with-libbind=$(EXTINSTDIR) \
	--without-randomdev \
	--enable-dhcpv6 \
	--disable-libtool

MICB_CONFIGURE_MAKEOPTS += \
	BUILD_CC=gcc \

MICB_DEPENDS = openssl libxml-2.0

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)
	@$(MICB_CONFIGURE_BUILDDIR_AUTOCONF) 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

