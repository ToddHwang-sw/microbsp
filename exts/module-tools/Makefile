DIR=module-init-tools-3.15
LOC=https://mirrors.edge.kernel.org/pub/linux/utils/kernel/module-init-tools/

include $(TOPDIR)/scripts/extenv.mk
ifeq ($(_CORE_),aarch64)
	MICB_CONFIGURE_PLATFORM=arm-linux
endif

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

