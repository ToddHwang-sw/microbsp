DIR=sassc
LOC=https://github.com/sass

LDIR=libsass
SDIR=sass-spec
SRCPATH=$(TOPDIR)/uix/sassc/$(MICBSRC)

include $(TOPDIR)/scripts/uixenv.mk

SASS_PATHENV=\
	SASS_SASSC_PATH=$(SRCPATH)/$(DIR) \
	SASS_LIBSASS_PATH=$(SRCPATH)/$(DIR)/$(LDIR) \
	SASS_SPEC_PATH=$(SRCPATH)/$(DIR)/$(SDIR) \
	BUILD=shared 

MICB_CONFIGURE_MAKEOPTS += -C ../../../$(MICBSRC)/$(DIR)

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( \
			$(UNCOMPRESS) $(LOC)/$(DIR).git;  \
			pushd $(SRCPATH)/$(DIR);                  \
				git clone --single-branch $(LOC)/$(LDIR).git; \
				git clone --single-branch $(LOC)/$(SDIR).git; \
			popd;                                     \
			$(MICB_PATCH) )

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(SASS_PATHENV) $(COMP_CMDS) make $(MICB_CONFIGURE_MAKEOPTS) $@ ; \
		[ "$@" != "install" ] || \
			cp -rf $(SRCPATH)/$(DIR)/$(LDIR)/src/support/$(LDIR).pc.in $(MICB_INSTALL_PLACE)/usr/local/lib/pkgconfig/$(LDIR).pc  )


