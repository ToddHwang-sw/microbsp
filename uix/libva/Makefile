DIR=libva
LOC=-b v2.13-branch https://github.com/intel

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS += \
		-D enable_docs=false \
		-D with_glx=no \
		-D with_wayland=yes

MICB_DEPENDS = libdrm_nouveau libdrm_freedreno libdrm_vc4 zlib xext xfixes x11 \
			   egl wayland-client

download:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )

