DIR=linuxptp
LOC=git://git.code.sf.net/p/linuxptp/code

include $(TOPDIR)/scripts/extenv.mk

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || ( \
		[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR); \
		cd $(BUILDDIR);                             \
		git clone $(LOC).git $(DIR) )
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		cat ../../../$(MICB_PATCH_FN) | patch -p 1 )

prepare:

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
         cd $(BUILDDIR)/$(DIR); \
		 make CROSS_COMPILE=$(CROSS_COMP_PREFIX) EXTRA_CFLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)" \
		 	EXTRA_LDFLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_LFLAGS)" \
			$(MICB_CONFIGURE_MAKEOPTS) $@ )

