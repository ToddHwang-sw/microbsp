DIR=drm
LOC=https://gitlab.freedesktop.org/mesa/

include $(TOPDIR)/scripts/extenv.mk

MICB_MESON_OPTS+=\
		-D wrap_mode=nodownload \
		-D valgrind=disabled    \
		-D cairo-tests=disabled \
		-D man-pages=disabled

ifeq ($(_CORE_),x86_64)
	MICB_DEPENDS = pciaccess
endif

download:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )

