DIR=libvncserver
LOC=https://github.com/LibVNC

include $(TOPDIR)/scripts/uixenv.mk

MICB_CMAKE_COMMON_OPTS += \
	-D WITH_TESTS=OFF    \
	-D LIBVNCSERVER_INSTALL=ON \
	-D ZLIB_LIBRARY=$(INSTALLDIR)/lib \
	-D LZO_LIBRARY=$(INSTALLDIR)/lib \
	-D LZO_INCLUDE_DIR=$(INSTALLDIR)/include/lzo \
	-D LIBSSH2_LIBRARY=$(INSTALLDIR)/lib \
	-D LIBSSH2_INCLUDE_DIR=$(INSTALLDIR)/include \
	-D JPEG_LIBRARY=$(UIXINSTDIR)/lib \
	-D JPEG_INCLUDE_DIR=$(UIXINSTDIR)/include \
	-D PNG_LIBRARY=$(UIXINSTDIR)/lib \
	-D PNG_INCLUDE_DIR=$(UIXINSTDIR)/include \
	-D PNG_PNG_INCLUDE_DIR=$(UIXINSTDIR)/include \
	-D X11_X11_INCLUDE_PATH=$(UIXINSTDIR)/include \
	-D X11_X11_LIB=$(UIXINSTDIR)/usr/lib \
	-D SDL2_LIBRARY=$(UIXINSTDIR)/lib \
	-D SDL2_INCLUDE_DIR=$(UIXINSTDIR)/include \
	-D FFMPEG_LIBRARY=$(UIXINSTDIR)/local/lib \
	-D FFMPEG_INCLUDE_DIR=$(UIXINSTDIR)/local/include \
	-D FFMPEG_LIBRARIES=$(UIXINSTDIR)/local/lib \
	-D FFMPEG_INCLUDE_DIRS=$(UIXINSTDIR)/local/include \
	-D GNUTLS_LIBRARY=$(EXTINSTDIR)/lib \
	-D GNUTLS_INCLUDE_DIR=$(EXTINSTDIR)/include \
	-D OPENSSL_ROOT_DIR=$(INSTALLDIR) \
	-D OPENSSL_LIBRARY=$(INSTALLDIR)/lib \
	-D OPENSSL_INCLUDE_DIR=$(INSTALLDIR)/include \
	-D SYSTEMD_LIBRARIES=$(UIXINSTDIR)/usr/lib \
	-D SYSTEMD_INCLUDE_DIRS=$(UIXINSTDIR)/usr/include \

MICB_DEPENDS = libjpeg libpng libpng16 sdl2 gnutls openssl x11 x11-xcb libdrm_freedreno libdrm_nouveau  \
	       freetype libgcrypt gstreamer-1.0 libssh2 \
	       expat libffi intl libudev uuid blkid waylandd-server egl gl glesv2 \
	       xcb-xfixes wayland-server xcb-dri3 xshmfence ncurses xcb-present xcb-sync xcb-randr \
	       libsystemd zlib libcap liblzo liblzma liblz4 libzstd \
		   libavsample libavcodec libavutil libavfilter libavformat 

CROSS_LFLAG_EXTRA += -L$(UIXINSTDIR)/lib -lglapi

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_CMAKE_RUNENV) cmake $(MICB_CMAKE_COMMON_OPTS) ../../../$(MICBSRC)/$(DIR) 
		
all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 $@ )

