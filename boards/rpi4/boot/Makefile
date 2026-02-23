UBOOTVER=v2025.10-rc5
DIR=u-boot-$(UBOOTVER)
LOC=https://gitlab.denx.de/u-boot/u-boot/-/archive/$(UBOOTVER)
UBOOTMAKE=make ARCH=arm V=1 -C . HOSTCC=gcc HOSTCXX=g++ CROSS_COMPILE=$(CROSS_COMP_PREFIX)
SRC=source

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR) 
	@[ -f $(BUILDDIR)/$(DIR)/Makefile ] || (  \
		[ -f $(SRC)/$(DIR).tar.bz2 ] || wget -P $(SRC) $(LOC)/$(DIR).tar.bz2; \
		tar jxvf $(SRC)/$(DIR).tar.bz2 -C $(BUILDDIR) > /dev/null ; \
		[ ! -f patch/uboot.config ] || (cp patch/uboot.config $(BUILDDIR)/$(DIR)/.config) ;          \
		[ ! -f patch/rpi.h        ] || (cp patch/rpi.h        $(BUILDDIR)/$(DIR)/include/configs) ;  \
		[ ! -d $(BUILDDIR)/$(DIR) ] || (cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) olddefconfig) ;          \
	)

config:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) menuconfig )

all clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) $@ )

install: 
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); cp -rf u-boot.bin $(isodir)/boot )

distclean:
	@\rm -rf $(BUILDDIR)/$(DIR)
