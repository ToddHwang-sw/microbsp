DIR=libgpiod
LOC=https://git.kernel.org/pub/scm/libs/libgpiod/

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_AUTOCONF_CMD=./autogen.sh

MICB_CONFIGURE_RUNENV += \
	ac_cv_func_malloc_0_nonnull=yes

MICB_CONFIGURE_OPTS += \
	--enable-tools \
	--disable-tests \
	--disable-examples 

MICB_CONFIGURE_PRG = ./configure 

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


