DIR=debian_u-boot
LOC=-b release https://github.com/TinkerBoard
UBOOTMAKE=make ARCH=arm -C . CROSS_COMPILE=$(CROSS_COMP_PREFIX)
UBOOTIMG=u-boot-bootable.img

RP=../../..

prepare:
	@[ -d $(BUILDDIR)/$(DIR)      ] || mkdir -p $(BUILDDIR)/$(DIR) 
	@[ -f $(BUILDDIR)/$(DIR)/.git ] || ( cd $(BUILDDIR); \
		git clone $(LOC)/$(DIR).git;       \
		cd $(DIR);                         \
		[ ! -f $(RP)/patch/patch        ] || cat $(RP)/patch/patch | patch -p 1 ;     \
		[ ! -f $(RP)/patch/uboot.config ] || cp  $(RP)/patch/uboot.config .config ;   \
		$(UBOOTMAKE) olddefconfig                                                  \
	)

config:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) menuconfig )

all clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) $@ )

install: 
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); \
		tools/mkimage -n rk3288 -T rksd -d spl/u-boot-spl-dtb.bin $(UBOOTIMG); \
		cat u-boot.bin >> $(UBOOTIMG); \
	    cp $(UBOOTIMG) $(isodir)/boot )

distclean:
	@\rm -rf $(BUILDDIR)/$(DIR)
