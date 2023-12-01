DIR=ncftp-3.2.6
LOC=https://www.ncftp.com/downloads/ncftp

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
	@[ -f $(MICBSRC)/$(DIR)-src.tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR)-src.tar.gz; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

