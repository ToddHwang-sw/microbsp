DIR=pulseaudio
LOC=-b stable-14.x https://gitlab.freedesktop.org/pulseaudio

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS+=\
	 -D man=false \
	 -D gcov=false \
	 -D tests=false \
	 -D database=simple \
	 -D bluez5=false  \
	 -D gstreamer=enabled 

MICB_DEPENDS = dbus-1 alsa sndfile x11-xcb x11 xcb xau libudev uuid   \
		libsystemd ncursesw libjpeg cairo-script libcap               \
		liblzma liblz4 libzstd libgcrypt libcrypto wayland-cursor     \
		json-c egl wayland-egl lzo2 formw menuw panelw sm xtst  \
		wayland-server xcb-dri2 xcb-dri3 xcb-present xcb-xfixes \
		gbm xcb-sync xshmfence \
		gstreamer-1.0       \
		gstreamer-base-1.0  \
		gstreamer-plugins-base-1.0 \
		gstreamer-audio-1.0        \
		gstreamer-bad-audio-1.0 

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

