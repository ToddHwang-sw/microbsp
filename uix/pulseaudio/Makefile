DIR=pulseaudio
LOC=-b stable-14.x https://gitlab.freedesktop.org/pulseaudio

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS+=\
	 -D doxygen=false \
	 -D man=false \
	 -D gcov=false \
	 -D tests=false \
	 -D database=simple \
	 -D bluez5=false  \
	 -D gstreamer=enabled 

MICB_DEPENDS = dbus-1 sndfile gstreamer-1.0 x11-xcb x11 xcb xau

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ]   || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); DESTDIR=$(destination) ninja -v $@ )

