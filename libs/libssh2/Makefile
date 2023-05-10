DIR=libssh2
LOC=https://github.com/libssh2

include $(TOPDIR)/scripts/env.mk

MICB_CMAKE_COMMON_OPTS +=             \
	-D OPENSSL_ROOT_DIR=$(INSTALLDIR) \
	-D ZLIB_LIBRARY=$(INSTALLDIR)     \
	-D BUILD_SHARED_LIBS=ON           \
	-D BUILD_EXAMPLES=OFF             \
	-D BUILD_TESTING=OFF

MICB_DEPENDS = openssl zlib

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_CMAKE_RUNENV) cmake $(MICB_CMAKE_COMMON_OPTS) ../../../$(MICBSRC)/$(DIR) 
		
all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 $@ )

