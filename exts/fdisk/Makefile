DIR=fdisk-1.3.0a
LOC=https://ftp.gnu.org/gnu/fdisk

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	ac_cv_lib_readline_readline=yes

MICB_CONFIGURE_OPTS += \
	--enable-largefile \
	--disable-cfdisk 

MICB_DEPENDS = libparted uuid blkid ncurses

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz ; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

