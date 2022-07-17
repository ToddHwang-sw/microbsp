DIR=libidn2
LOC=https://gitlab.com/libidn

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_AUTOCONF_CMD = ./bootstrap --force

MICB_CONFIGURE_OPTS += \
	--disable-doc \
	--disable-nls \
	--disable-valgrind-tests \
	--disable-valgrind-script \
	--disable-silent-rules

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ -f $(BUILDDIR)/$(DIR)/$(BSDONE_FILE) ] || ( \
		$(MICB_CONFIGURE_AUTOCONF) ; \
		touch $(BUILDDIR)/$(DIR)/$(BSDONE_FILE) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )
