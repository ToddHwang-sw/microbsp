UBOOTVER=v2022.10-rc3
DIR=u-boot-$(UBOOTVER)
LOC=https://gitlab.denx.de/u-boot/u-boot/-/archive/$(UBOOTVER)
DEFCONFIG=rpi_3_b_plus_defconfig
UBOOTMAKE=make ARCH=arm -C . CROSS_COMPILE=$(CROSS_COMP_PREFIX)
SRC=source

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR) 
	@[ -f $(BUILDDIR)/$(DIR)/Makefile ] || (  \
		[ -f $(SRC)/$(DIR).tar.bz2 ] || wget -P $(SRC) $(LOC)/$(DIR).tar.bz2; \
		tar jxvf $(SRC)/$(DIR).tar.bz2 -C $(BUILDDIR) > /dev/null ; \
		[ ! -f patch/patch        ] || (cd $(BUILDDIR); cat ../../patch/patch | patch -p 1) ;    \
		[ ! -f patch/uboot.config ] || (cp patch/uboot.config $(BUILDDIR)/$(DIR)/.config ) ;  \
	)
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) olddefconfig )

config:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) menuconfig )

all clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) $@ )

install: 
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); cp -rf u-boot.bin $(isodir)/boot )

distclean:
	@\rm -rf $(BUILDDIR)/$(DIR)
