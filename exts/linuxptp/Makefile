DIR=linuxptp
LOC=https://github.com/richardcochran

include $(TOPDIR)/scripts/extenv.mk

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)

prepare:

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
         cd $(BUILDDIR)/$(DIR); \
		 make CROSS_COMPILE=$(CROSS_COMP_PREFIX) EXTRA_CFLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)" \
		 	EXTRA_LDFLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_LFLAGS)" \
			$(MICB_CONFIGURE_MAKEOPTS) $@ )

