DIR=ntpclient
LOC=https://github.com/lettier

include $(TOPDIR)/scripts/extenv.mk

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@cp -rf $(MICBSRC)/$(DIR)/* $(BUILDDIR)/$(DIR) 

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(COMP_CMDS) $(MICB_CONFIGURE_MAKEOPTS) $@ )


