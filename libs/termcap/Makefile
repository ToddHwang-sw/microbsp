DIR=termcap-1.3.1
LOC=ftp://ftp.gnu.org/gnu/termcap

HOSTOPT=$(PLATFORM)
ifeq ($(_CORE_),aarch64)
	MICB_CONFIGURE_PLATFORM=arm-linux
endif

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS= prefix=$(MICB_INSTALL_PLACE)

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )

prepare: 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

