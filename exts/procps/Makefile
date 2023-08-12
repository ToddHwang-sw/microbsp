DIR=procps
LOC=http://gitlab.com/procps-ng
	
include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_AUTOCONF_CMD = \
	autoreconf --install -v -I$(INSTALLDIR) -I$(EXTINSTDIR) 

MICB_CONFIGURE_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) \
	$(COMP_CMDS) \
	ac_cv_func_malloc_0_nonnull=yes \
	ac_cv_func_realloc_0_nonnull=yes

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_OPTS += \
	--enable-largefile \
	--with-ncurses 

MICB_DEPENDS = ncurses

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)

prepare:
	@$(MICB_CONFIGURE_BUILDDIR_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean sync-with-kernel :
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) V=1 $@ )

