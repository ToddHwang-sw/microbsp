DIR=nftables-0.9.7
LOC=http://www.netfilter.org/projects/nftables/files
	
include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	ac_cv_lib_readline_readline=yes

MICB_CONFIGURE_OPTS += \
	--disable-man-doc \
	--disable-python

MICB_DEPENDS = openssl libmnl libnftnl ncurses

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.bz2 ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2 ; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

