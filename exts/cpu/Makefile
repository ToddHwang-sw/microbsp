DIR=cpu

include $(TOPDIR)/scripts/extenv.mk

download:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(DIR) ] || tar jxvf $(MICBSRC)/$(DIR).tar.bz2 -C $(BUILDDIR) 

prepare:

all install uninstall clean:
	[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		make $(COMP_CMDS) DESTDIR=$(MICB_INSTALL_PLACE)/usr $@ )

wipeout:
