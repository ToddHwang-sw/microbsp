DIR=pkg-config-0.29.1
LOC=https://pkg-config.freedesktop.org/releases/

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	glib_cv_stack_grows=yes \
	glib_cv_uscore=no \
	ac_cv_func_posix_getpwuid_r=no \
	ac_cv_func_posix_getgrgid_r=no 

MICB_DEPENDS = intl iconv glib-2.0 libffi

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) V=1 $@ )

