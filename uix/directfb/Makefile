DIR=directfb
LOC=https://github.com/DirectFB

FLUXDIR=flux
FLUXLOC=https://github.com/deniskropp/$(FLUXDIR).git
FLUXCOMPAPP=$(TOPDIR)/uix/directfb/$(BUILDDIR)/$(FLUXDIR)/src/fluxcomp

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_RUNENV += \
	ac_cv_path_FLUXCOMP=$(FLUXCOMPAPP)

MICB_CONFIGURE_OPTS += \
	--enable-maintainer-mode \
	--enable-mesa \
	--enable-gstreamer \
	--enable-ffmpeg \
	--disable-egl \
	--disable-idirectfbgl-egl

MICB_DEPENDS = expat intl iconv gio-unix-2.0 gstreamer-1.0 x11 xext libdrm libxml-2.0 libpng libpng16 freetype2 \
	       wayland-client wayland-server gl egl xcb xshmfence xcb-present xcb-dri3 

CROSS_USER_CFLAGS += -lglapi

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

