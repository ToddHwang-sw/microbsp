DIR=fontconfig
LOC=https://gitlab.freedesktop.org/fontconfig

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_RUNENV += \
	CC_FOR_BUILD=gcc 

MICB_CONFIGURE_OPTS += \
	--enable-largefile \
	--disable-docs 

MICB_DEPENDS = freetype2 json-c libpng zlib expat

prepare: 
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

