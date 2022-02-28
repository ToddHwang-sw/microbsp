DIR=zip
LOC=https://github.com/LuaDist

include $(TOPDIR)/scripts/extenv.mk

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		cmake $(MICB_CMAKE_COMMON_OPTS) ../../../$(MICBSRC)/$(DIR) 
		
all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 $@ )

