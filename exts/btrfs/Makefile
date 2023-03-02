DIR=btrfs-progs
LOC=https://github.com/kdave

include $(TOPDIR)/scripts/extenv.mk

CROSS_USER_CFLAGS += \
	-DMB_LEN_MAX=16 \
	-DULLONG_MAX=18446744073709551615ULL

MICB_CONFIGURE_AUTOCONF_CMD = ./autogen.sh

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_OPTS += \
	--disable-documentation --enable-programs --enable-largefile --disable-backtrace \
	--disable-python --disable-libudev --with-convert=ext2 

MICB_CONFIGURE_MAKEOPTS += V=1 

##
## Originally, btrfs searches SSE feature, but it cannot go with AARCH64 platform. 
## This file will be deleted. 
##
BTRFS_TAILOR_DELETE=m4/ax_gcc_builtin.m4

MICB_DEPENDS = lzo2 uuid blkid libzstd

prepare:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || \rm -rf $(BUILDDIR)/$(DIR)
	@mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)
	@cd $(BUILDDIR)/$(DIR); \
		[ ! -f $(BTRFS_TAILOR_DELETE) ] || \rm -rf $(BTRFS_TAILOR_DELETE)
	@$(MICB_CONFIGURE_BUILDDIR_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

