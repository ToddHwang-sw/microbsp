UBOOT=u-boot-v2020.07-rc3
DEFCONFIG=rpi_3_defconfig
FILENM=$(UBOOT).tar.bz2
LOC=https://gitlab.denx.de/u-boot/u-boot/-/archive/v2020.07-rc3/$(FILENM)

UBOOTMAKE=make -C . CROSS_COMPILE=$(CROSS_COMP_PREFIX)

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(UBOOT) ] || ( \
		[ -f source/$(FILENM) ] || ( cd source; wget $(LOC) ) )
	@[ -d $(BUILDDIR)/$(UBOOT) ] || (                          \
		tar jxvf source/$(FILENM) -C $(BUILDDIR) ;             \
		cp patch/patch         $(BUILDDIR)/$(UBOOT)/ ;         \
		cp patch/uboot.config  $(BUILDDIR)/$(UBOOT)/.config ;  \
		cd $(BUILDDIR)/$(UBOOT);                               \
		cat patch | patch -p 1 ;                               \
		$(UBOOTMAKE) $(DEFCONFIG) )

all clean:
	@[ ! -d $(BUILDDIR)/$(UBOOT) ] || ( \
		cd $(BUILDDIR)/$(UBOOT); \
		$(UBOOTMAKE) $@ )

install: 
	[ ! -d $(BUILDDIR)/$(UBOOT) ] || ( \
		cd $(BUILDDIR)/$(UBOOT); \
		cp -rf u-boot.bin $(isodir)/boot )

distclean:
	@\rm -rf $(BUILDDIR)/$(UBOOT)

config:
	@[ ! -d $(BUILDDIR)/$(UBOOT) ] || (     \
		cd $(BUILDDIR)/$(UBOOT);      \
		$(UBOOTMAKE) menuconfig )
