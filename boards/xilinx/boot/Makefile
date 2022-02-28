UBOOT=u-boot-xlnx-master
DEFCONFIG=xilinx_zynqmp_mini_qspi_defconfig
FILENM=master.zip
LOC=https://github.com/Xilinx/u-boot-xlnx/archive/$(FILENM)

UBOOTMAKE=make -C . CROSS_COMPILE=$(CROSS_COMP_PREFIX)

## Squash FS image ..
RAMFSIMAGE=arch/arm64/ramdisk/ramdisk.squash

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(UBOOT) ] || ( \
		[ -f source/$(FILENM) ] || ( cd source; wget $(LOC) ) )
	@[ -d $(BUILDDIR)/$(UBOOT) ] || (                          \
		$(UNCOMPRESS) source/$(FILENM) $(BUILDDIR) ;           \
		cd $(BUILDDIR)/$(UBOOT);                               \
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
