DIR=libepoxy
LOC=https://github.com/anholt

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
        -D docs=false \
        -D tests=false

MICB_DEPENDS = x11 gl egl

CROSS_USER_LFLAGS += -lglapi

download:
	@[ -d $(BUILDDIR)/$(DIR)] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )


