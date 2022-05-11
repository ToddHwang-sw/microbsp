DIR=libxkbcommon
LOC=https://github.com/xkbcommon

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
	-D enable-docs=false \
	-D base-install=$(UIXINSTDIR)

CROSS_USER_CFLAGS += -DLONG_BIT=32

MICB_DEPENDS = libxml-2.0 libffi xcb-xkb xcb-x11

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )

