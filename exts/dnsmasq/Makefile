DIR=dnsmasq-2.80
LOC=https://www.thekelleys.org.uk/dnsmasq

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS) PREFIX=$(USRNODE)

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz ; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)

prepare:

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

