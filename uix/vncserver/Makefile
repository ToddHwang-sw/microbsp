DIR=libvncserver
LOC=https://github.com/LibVNC

include $(TOPDIR)/scripts/uixenv.mk

MICB_CMAKE_COMMON_OPTS += \
	-D LIBVNCSERVER_INSTALL=ON \
	-D ZLIB_LIBRARY=$(INSTALLDIR)/lib \
	-D LZO_LIBRARY=$(INSTALLDIR)/lib \
	-D LZO_INCLUDE_DIR=$(INSTALLDIR)/include/lzo \
	-D JPEG_LIBRARY=$(UIXINSTDIR)/lib \
	-D JPEG_INCLUDE_DIR=$(UIXINSTDIR)/include \
	-D PNG_LIBRARY=$(UIXINSTDIR)/lib \
	-D PNG_INCLUDE_DIR=$(UIXINSTDIR)/include \
	-D PNG_PNG_INCLUDE_DIR=$(UIXINSTDIR)/include \
	-D SDL2_LIBRARY=$(UIXINSTDIR)/lib \
	-D SDL2_INCLUDE_DIR=$(UIXINSTDIR)/include \
	-D FFMPEG_LIBRARY=$(UIXINSTDIR)/usr/local/lib \
	-D FFMPEG_INCLUDE_DIR=$(UIXINSTDIR)/usr/local/include \
	-D FFMPEG_LIBRARIES=$(UIXINSTDIR)/usr/local/lib \
	-D FFMPEG_INCLUDE_DIRS=$(UIXINSTDIR)/usr/local/include \
	-D GNUTLS_LIBRARY=$(EXTINSTDIR)/lib \
	-D GNUTLS_INCLUDE_DIR=$(EXTINSTDIR)/include \
	-D OPENSSL_ROOT_DIR=$(INSTALLDIR) \
	-D OPENSSL_LIBRARY=$(INSTALLDIR)/lib \
	-D OPENSSL_INCLUDE_DIR=$(INSTALLDIR)/include \
	-D SYSTEMD_LIBRARIES=$(UIXINSTDIR)/usr/lib \
	-D SYSTEMD_INCLUDE_DIRS=$(UIXINSTDIR)/usr/include \


MICB_DEPENDS = libjpeg libpng libpng16 sdl2 gnutls openssl x11 libdrm_freedreno libdrm_nouveau  \
	       expat libffi intl libudev uuid blkid waylandd-server egl gl glesv2 \
	       xcb-xfixes wayland-server xcb-dri3 xshmfence ncurses xcb-present xcb-sync xcb-randr \
	       libsystemd zlib libcap liblzo liblzma liblz4 libzstd \
		   libavsample libavcodec libavutil libavfilter libavformat 

CROSS_USER_LFLAGS += -lglapi

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	cd $(BUILDDIR)/$(DIR); \
		$(MICB_CMAKE_RUNENV) cmake $(MICB_CMAKE_COMMON_OPTS) ../../../$(MICBSRC)/$(DIR) 
		
all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 $@ )

