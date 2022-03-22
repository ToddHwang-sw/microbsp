DIR=btrfs-progs
LOC=-b v5.16.x https://github.com/kdave

include $(TOPDIR)/scripts/extenv.mk

CROSS_USER_CFLAGS += \
	-DMB_LEN_MAX=16 \
	-DULLONG_MAX=18446744073709551615ULL

MICB_CONFIGURE_AUTOCONF_CMD = ./autogen.sh

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_OPTS += \
	--disable-documentation --enable-programs --enable-largefile --disable-backtrace --disable-zstd \
		--disable-python --disable-libudev --with-convert=ext2 

MICB_CONFIGURE_MAKEOPTS += V=1 

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)
	@$(MICB_CONFIGURE_BUILDDIR_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

