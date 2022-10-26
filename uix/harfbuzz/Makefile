BUZZVER=5.3.0
DIR=harfbuzz-$(BUZZVER)
LOC=https://github.com/harfbuzz/harfbuzz/releases/download/$(BUZZVER)

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D icu=disabled \
        -D docs=disabled \
        -D tests=disabled \
        -D introspection=disabled \
        -D icu=disabled \
        -D icu_builtin=false

MICB_DEPENDS = glib-2.0 freetype2 cairo-ft

CROSS_USER_LFLAGS += -lglapi

prepare:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )


