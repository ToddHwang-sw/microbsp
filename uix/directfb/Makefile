DIR=directfb
LOC=https://github.com/DirectFB

FLUXDIR=flux
FLUXLOC=https://github.com/deniskropp/$(FLUXDIR).git
FLUXCOMPAPP=$(TOPDIR)/uix/directfb/$(BUILDDIR)/$(FLUXDIR)/src/fluxcomp

include $(TOPDIR)/scripts/uixenv.mk

UILIBS += -lpng

MICB_CONFIGURE_RUNENV += \
	ac_cv_path_FLUXCOMP=$(FLUXCOMPAPP)

MICB_CONFIGURE_OPTS += \
	--enable-maintainer-mode \
	--enable-mesa \
	--enable-gstreamer \
	--enable-ffmpeg \
	--disable-egl \
	--disable-idirectfbgl-egl

MICB_CONFIGURE_MAKEOPTS += V=1 

fluxcomp:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(FLUXDIR) ] || ( cd $(BUILDDIR); $(UNCOMPRESS) $(FLUXLOC) )
	@[ -f $(FLUXCOMPAPP) ] || ( \
		cd $(BUILDDIR)/$(FLUXDIR); \
		./autogen.sh ; ./configure --prefix= ; make )

prepare: fluxcomp
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

