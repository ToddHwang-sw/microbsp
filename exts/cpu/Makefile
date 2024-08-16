DIR=cpu

include $(TOPDIR)/scripts/extenv.mk

download:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)

prepare:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || \rm -rf $(BUILDDIR)/$(DIR)
	@[ -d $(BUILDDIR)/$(DIR) ]   || tar jxvf $(MICBSRC)/$(DIR).tar.bz2 -C $(BUILDDIR) 

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		make $(COMP_CMDS) DESTDIR=$(MICB_INSTALL_PLACE)/usr $@ )

wipeout:
