DIR=npd6
LOC=https://github.com/npd6

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS) 

prepare:
	[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH))
	$(MICB_DUP_SOURCES)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

