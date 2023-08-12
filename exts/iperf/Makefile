DIR=iperf-2.0.4
LOC=https://iperf.fr/download/source

include $(TOPDIR)/scripts/extenv.mk

ifeq ($(_CORE_),aarch64)
	MICB_CONFIGURE_PLATFORM=arm-linux
endif

MICB_CONFIGURE_RUNENV += \
	ac_cv_func_malloc_0_nonnull=yes

MICB_CONFIGURE_OPTS += --enable-ipv6

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR)-source.tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR)-source.tar.gz; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

