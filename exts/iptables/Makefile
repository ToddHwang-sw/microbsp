DIR=iptables-1.8.5
LOC=http://www.netfilter.org/projects/iptables/files
	
include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH)

MICB_CONFIGURE_PRG = ./configure 

MICB_DEPENDS = libnftnl

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.bz2 ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

