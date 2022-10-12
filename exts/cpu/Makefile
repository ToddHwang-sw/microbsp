DIR=cpu

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(DIR) ] || tar jxvf $(MICBSRC)/$(DIR).tar.bz2 -C $(BUILDDIR) 

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		make $(COMP_CMDS) $(MICB_CONFIGURE_MAKEOPTS) $@ )

wipeout:
