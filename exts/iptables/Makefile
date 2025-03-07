DIR=iptables-1.8.5
LOC=http://www.netfilter.org/projects/iptables/files
	
include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_PRG = ./configure 

MICB_CONFIGURE_OPTS += \
	--with-kernel=$(KERNDIR)/$(KERNELVER) 

MICB_DEPENDS = libnftnl libmnl

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.bz2 ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2; $(MICB_PATCH) )

##
## "iptables" depends on linux kernel header ..
##
kernel_setup:
	@make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) prepare

prepare: kernel_setup
	@$(MICB_DUP_SOURCES)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

