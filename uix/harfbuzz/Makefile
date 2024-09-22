BUZZVER=5.3.0
DIR=harfbuzz-5.3.0
##
## We cannot use a symbol like "DIR=harfbuzzx-$(BUZZVER)" - conflicts with 'distclean' rule.
##
LOC=https://github.com/harfbuzz/harfbuzz/releases/download/$(BUZZVER)

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D icu=disabled \
    -D docs=disabled \
    -D tests=disabled \
    -D introspection=disabled \
    -D icu_builtin=false

MICB_DEPENDS = glib-2.0 gobject-2.0 freetype2 cairo-ft

download:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.xz   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.xz ; $(MICB_PATCH) )

prepare:
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )


