DIR=fdisk-1.1
LOC=ftp://ftp.gnu.org/gnu/fdisk
FILENM=$(DIR).tar.gz

include $(TOPDIR)/scripts/extenv.mk

ifeq ($(_CORE_),aarch64)
	MICB_CONFIGURE_PLATFORM=arm-linux
endif

MICB_CONFIGURE_RUNENV += \
	ac_cv_lib_readline_readline=yes \
	ac_cv_lib_parted_ped_device_read=yes \

MICB_CONFIGURE_OPTS += \
	--enable-largefile

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz ; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


