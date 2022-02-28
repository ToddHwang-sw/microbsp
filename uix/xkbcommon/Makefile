DIR=libxkbcommon
LOC=https://github.com/xkbcommon

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D enable-docs=false \
	-D enable-wayland=false

CROSS_USER_CFLAGS += -DLONG_BIT=sizeof(long)
CROSS_LFLAG_EXTRA += $(LIBS_BASIC)

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )

