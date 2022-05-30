DIR=xconfig

include $(TOPDIR)/scripts/env.mk

## 2 GBytes 
CROSS_USER_CFLAGS += \
	-DSSIZE_MAX=2147483647

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

wipeout:

