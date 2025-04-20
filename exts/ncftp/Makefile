DIR=ncftp
LOC=https://github.com/deepin-community

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_PRG = ./configure 

##
## don't know why these are needed ..
##
MICB_CONFIGURE_MAKEOPTS += \
		STRIP=$(CROSS_COMP_PREFIX)strip \
		AR=$(CROSS_COMP_PREFIX)ar

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

