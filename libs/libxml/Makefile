DIR=libxml2
LOC=https://github.com/GNOME

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_AUTOCONF_CMD=\
	NOCONFIGURE=0 ./autogen.sh 

MICB_CONFIGURE_OPTS += \
	--without-python

MICB_DEPENDS = zlib liblzma

prepare: 
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF) 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


