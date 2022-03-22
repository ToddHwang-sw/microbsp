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

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	cd $(BUILDDIR)/$(DIR); \
		$(MICB_CMAKE_RUNENV) cmake $(MICB_CMAKE_COMMON_OPTS) ../../../$(MICBSRC)/$(DIR) 
		
all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 $@ )

