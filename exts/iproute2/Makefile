DIR=iproute2-5.4.0
LOC=https://mirrors.edge.kernel.org/pub/linux/utils/net/iproute2/

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += PKG_CONFIG=""

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_MAKEOPTS += V=1

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

