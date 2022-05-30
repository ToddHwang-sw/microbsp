DIR=libpciaccess
LOC=https://gitlab.freedesktop.org/xorg/lib

include $(TOPDIR)/scripts/extenv.mk

MICB_MESON_OPTS+=-D wrap_mode=nodownload

MICB_DEPENDS = zlib

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ]  || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ]  || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@[ "$(_CORE_)" != "x86_64" ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS) )

all install uninstall clean:
	@[ "$(_CORE_)" != "x86_64" ] || ( \
		[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ ) \
	) 

