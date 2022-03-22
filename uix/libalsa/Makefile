DIR=alsa-lib
LOC=-b ucm2 https://github.com/alsa-project

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_OPTS +=\
	--disable-debug \
	--enable-mixer \
	--enable-pcm \
	--disable-hwdep \
	--enable-rawmidi \
	--enable-seq \
	--enable-topology

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

