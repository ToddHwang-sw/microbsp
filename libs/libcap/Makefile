DIR=libcap
LOC=https://git.kernel.org/pub/scm/linux/kernel/git/morgan

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS) BUILD_CC=gcc LD="$(CROSS_COMP_PREFIX)gcc -shared" WARNINGS=

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR); git clone $(LOC)/$(DIR).git )

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


