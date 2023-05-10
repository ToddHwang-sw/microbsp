DIR=ffmpeg
LOC=-b release/4.4 https://git.ffmpeg.org

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_OPTS = \
	--prefix= \
	--enable-cross-compile \
	--arch=$(_CORE_) \
	--cpu=armv8-a \
	--target-os=linux \
	--disable-doc \
	--disable-symver \
	--disable-static \
	--enable-shared \
	--enable-neon \
	--enable-avfilter \
	--enable-avcodec \
	--enable-avdevice \
	--enable-avformat \
	--enable-avresample \
	--enable-avutil \
	--enable-postproc \
	--enable-swresample \
	--enable-swscale \
	--enable-shared \
	--extra-cflags="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)" \
	--extra-ldflags="$(CROSS_COMP_FLAGS) $(CROSS_USER_LFLAGS) $(CROSS_LFLAGS_EXTRA)" \
	--cross-prefix=$(CROSS_COMP_PREFIX) \
	--pkg-config=$(MICB_MESON_PKG_CONFIG) \
	--enable-gpl \
	--enable-nonfree

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_RUNENV) $(MICB_CONFIGURE_PRG) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


