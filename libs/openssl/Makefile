DIR=openssl-1.1.1c
LOC=https://www.openssl.org/source/

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_CMD=../../../$(MICBSRC)/$(DIR)/Configure 
MICB_CONFIGURE_OPTS=--prefix=/ --openssldir=/etc/openssl shared no-async linux-$(_CORE_)
MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS)

prepare: 
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) )

install:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) install_sw )

clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) clean )

uninstall:

