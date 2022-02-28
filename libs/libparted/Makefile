DIR=parted-1.8.0
LOC=https://ftp.gnu.org/gnu/parted

include $(TOPDIR)/scripts/env.mk

HOSTOPT=$(PLATFORM)
ifeq ($(_CORE_),aarch64)
	MICB_CONFIGURE_PLATFORM=arm-linux
endif

MICB_CONFIGURE_OPTS += \
	--without-readline \
	--disable-device-mapper 

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


