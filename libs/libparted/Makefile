DIR=parted-3.5
LOC=https://ftp.gnu.org/gnu/parted

include $(TOPDIR)/scripts/env.mk

ifeq ($(_CORE_),aarch64)
	MICB_CONFIGURE_PLATFORM=arm-linux
endif

MICB_CONFIGURE_OPTS += \
	--with-readline \
	--disable-device-mapper \
	--enable-fs \
	--enable-largefile \
	--enable-read-only

MICB_DEPENDS = ncurses uuid blkid

CROSS_USER_CFLAGS += \
	-D MB_LEN_MAX=16 

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz; $(MICB_PATCH) )

prepare: 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


