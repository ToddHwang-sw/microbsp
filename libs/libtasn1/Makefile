DIR=libtasn1
LOC=https://github.com/gnutls

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_OPTS += \
  --disable-fuzzing \
  --disable-doc \
  --disable-gtk-doc \
  --enable-year2038 \
  --enable-largefile

MICB_CONFIGURE_AUTOCONF_CMD = $(MICB_BOOTSTRAP_CMD)

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare: 
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

