DIR=linuxptp
LOC=https://github.com/richardcochran

include $(TOPDIR)/scripts/extenv.mk

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || ( \
		[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR) ; \
		$(TOPDIR)/scripts/setupsrcs.sh $(BUILDDIR)/$(DIR) $(LOC)/$(DIR).git  )
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		[ ! -f ../../../$(MICB_PATCH_FN) ] || (cat ../../../$(MICB_PATCH_FN) | patch -p 1) )

prepare:

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
         cd $(BUILDDIR)/$(DIR); \
		 make CROSS_COMPILE=$(CROSS_COMP_PREFIX) EXTRA_CFLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)" \
		 	EXTRA_LDFLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_LFLAGS)" \
			$(MICB_CONFIGURE_MAKEOPTS) $@ )

