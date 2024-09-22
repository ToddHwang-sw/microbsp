DIR=libxml2
LOC=https://github.com/GNOME

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_PRG=./configure

MICB_CONFIGURE_AUTOCONF_CMD=\
	NOCONFIGURE=1 ./autogen.sh 

MICB_CONFIGURE_OPTS += \
	--without-python

MICB_DEPENDS = zlib liblzma

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare: 
	@$(MICB_CONFIGURE_AUTOCONF) 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


