DIR=u-boot
LOC=-b v2023.10 https://source.denx.de/u-boot
UBOOTMAKE=make ARCH=arm -C . CROSS_COMPILE=$(CROSS_COMP_PREFIX)
UBOOTIMG=u-boot.img

## Please check this up if changed...
UBOOT_TEXTBASE=`cat ../$(DIR)/.config | grep CONFIG_TEXT_BASE | sed -e 's/CONFIG_TEXT_BASE=//'`

## Rockchip binary tools
RKBIN=rkbin
RKBINLOC=https://github.com/rockchip-linux

##
## Currently we support RK binary driven booting..
## U-boot base SPL/TPL is not yet supported - 04/16/2024
##
RKBINDDR=./bin/rk32/rk3288_ddr_400MHz_v1.11.bin
RKBINMINI=./bin/rk32/rk3288_miniloader_v2.63.bin
IDBLOADER=./idbloader.img

RP=../../..

prepare:
	@[ -d $(BUILDDIR)/$(DIR)      ] || mkdir -p $(BUILDDIR)/$(DIR) 
	@[ -f $(BUILDDIR)/$(DIR)/.git ] || ( cd $(BUILDDIR); \
		git clone --depth 1 $(LOC)/$(DIR).git;       \
		cd $(DIR);                         \
		[ ! -f $(RP)/patch/patch           ] || cat $(RP)/patch/patch | patch -p 1 ;     \
		[ ! -f $(RP)/patch/uboot.config    ] || cp  $(RP)/patch/uboot.config .config ;   \
		[ ! -f $(RP)/patch/tinker_rk3288.h ] || cp  $(RP)/patch/tinker_rk3288.h include/configs ;   \
		$(UBOOTMAKE) olddefconfig                                                  \
	)

config:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) menuconfig )

all clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR)/$(DIR); $(UBOOTMAKE) $@ )

install:
	@[ -d $(BUILDDIR)/$(RKBIN)  ] || (               \
		cd $(BUILDDIR);                              \
		git clone $(RKBINLOC)/$(RKBIN).git $(RKBIN)  )
	@[ ! -d $(BUILDDIR)/$(RKBIN)  ] || (             \
		cd $(BUILDDIR)/$(RKBIN) ;                    \
		./tools/mkimage -n rk3288 -T rksd -d $(RKBINDDR) $(isodir)/boot/$(IDBLOADER) ;        \
		cat $(RKBINMINI) >> $(isodir)/boot/$(IDBLOADER) ;                                     \
		./tools/loaderimage --pack --uboot ../$(DIR)/u-boot-dtb.bin $(isodir)/boot/$(UBOOTIMG) $(UBOOT_TEXTBASE))

distclean:
	@\rm -rf $(BUILDDIR)/$(DIR)
	@\rm -rf $(BUILDDIR)/$(RKBIN)
