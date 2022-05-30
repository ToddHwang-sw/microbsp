DIR=libnftnl
LOC=-b libnftnl-1.2.1 git://git.netfilter.org
	
include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_AUTOCONF_CMD=./autogen.sh

MICB_CONFIGURE_PRG=./configure 

MICB_DEPENDS = libmnl

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(DIR)/.git ] || ( cd $(BUILDDIR) ; git clone --single-branch $(LOC)/$(DIR).git )
	@$(MICB_CONFIGURE_BUILDDIR_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

