DIR=harfbuzz
LOC=https://github.com/harfbuzz

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D icu=disabled \
        -D docs=disabled \
        -D tests=disabled \
        -D introspection=disabled 

MICB_DEPENDS = glib-2.0 freetype2 cairo-ft

CROSS_USER_LFLAGS += -lglapi

prepare:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )


