DIR=fontconfig
LOC=https://github.com/freedesktop

include $(TOPDIR)/scripts/uixenv.mk

UILIBS += -lpng -lfreetype

CROSS_USER_CFLAGS += \
	-I$(EXTINSTDIR)/include/json-c

MICB_CONFIGURE_RUNENV += \
	CC_FOR_BUILD=gcc 

MICB_CONFIGURE_OPTS += \
	--enable-largefile \
	--disable-docs 

MICB_CONFIGURE_MAKEOPTS += V=1 

prepare: 
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

