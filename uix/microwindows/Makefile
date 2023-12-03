DIR=microwindows
LOC=https://github.com/ghaerr

##
## Sample image file & Image viewer 
##
SAMPLEIMAGE=patch/karma.jpg
IMAGEVIEW=src/bin/nxview

include $(TOPDIR)/scripts/uixenv.mk

MICB_DEPENDS = freetype2 libpng libpng16 libjpeg zlib

MICB_CONFIGURE_MAKEOPTS += \
	VERBOSE=Y \
	COMPILER="$(CROSS_COMP_PREFIX)gcc $(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS) $(CROSS_USER_LFLAGS)" \
	CXX_COMPILER="$(CROSS_COMP_PREFIX)g++ $(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS) $(CROSS_USER_LFLAGS)" \
	ARCHIVER="$(CROSS_COMP_PREFIX)ar" \
	LINKER="$(CROSS_COMP_PREFIX)ld" \
	EXTENGINELIBS="$(LIBS_BASIC) $(LIBS_GLIB) $(LIBS_UIBASIC)" \
	INSTALL_PREFIX=$(UIXINSTDIR) \
	INSTALL_OWNER1=  \
	INSTALL_OWNER2=  \
	MW_RULE_DEPEND_C= \
	CONFIG="$(TOPDIR)/uix/microwindows/$(BUILDDIR)/$(DIR)/src/Configs/config.raspberrypi" $@ \

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || ( \
		[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR); \
		$(TOPDIR)/scripts/setupsrcs.sh $(BUILDDIR)/$(DIR) $(LOC)/$(DIR).git; \
		[ ! -f $(MICB_PATCH_FN) ] || ( \
			cd $(BUILDDIR)/$(DIR); cat ../../../$(MICB_PATCH_FN) | patch -p 1 ) \
	)

prepare: 

all:
	@[ ! -d $(BUILDDIR)/$(DIR)/src ] || ( \
                cd $(BUILDDIR)/$(DIR)/src ; make $(MICB_CONFIGURE_MAKEOPTS) )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR)/src ] || ( \
                cd $(BUILDDIR)/$(DIR)/src ; make $(MICB_CONFIGURE_MAKEOPTS) $@ )
	@[ "$@" != "install" ] || ( \
		[ -d $(MICB_INSTALL_PLACE)/root ] || mkdir -p $(MICB_INSTALL_PLACE)/root ;                                      \
		[ ! -f $(BUILDDIR)/$(DIR)/$(IMAGEVIEW) ] || ( cp $(BUILDDIR)/$(DIR)/$(IMAGEVIEW) $(MICB_INSTALL_PLACE)/root ) ; \
		[ ! -f $(SAMPLEIMAGE) ] || ( cp $(SAMPLEIMAGE) $(MICB_INSTALL_PLACE)/root ) )

