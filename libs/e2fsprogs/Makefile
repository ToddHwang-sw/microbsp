DIR=e2fsprogs-1.45.3
LOC=https://sourceforge.net/projects/e2fsprogs/files/e2fsprogs/v1.45.3

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_OPTS += \
	--disable-e2initrd-helper \
	--disable-debugfs \
	--disable-imager \
	--disable-resizer \
	--disable-defrag \
	--disable-hardening \
	--disable-libuuid \
	--disable-libblkid \
	--disable-profile \
	--disable-ubsan \
	--disable-addrsan \
	--disable-threadsan

MICB_CONFIGURE_RUNENV += \
	ac_cv_func_mallinfo=no 

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

