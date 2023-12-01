DIR=e2fsprogs
LOC=https://github.com/tytso

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_OPTS += \
	--disable-e2initrd-helper \
	--disable-debugfs \
	--disable-imager \
	--disable-resizer \
	--disable-defrag \
	--disable-hardening \
	--disable-libblkid \
	--disable-profile \
	--disable-ubsan \
	--disable-addrsan \
	--disable-threadsan

MICB_CONFIGURE_RUNENV += \
	ac_cv_func_mallinfo=no 

MICB_DEPENDS = uuid blkid ncurses

download_ss:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare: 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

