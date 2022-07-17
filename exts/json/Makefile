DIR=json-c
LOC=https://github.com/json-c

include $(TOPDIR)/scripts/extenv.mk

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		cmake $(MICB_CMAKE_COMMON_OPTS) ../../../$(MICBSRC)/$(DIR) 
		
all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 $@ )

