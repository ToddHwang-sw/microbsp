DIR=libvncserver
LOC=https://github.com/LibVNC

include $(TOPDIR)/scripts/uixenv.mk

MICB_CMAKE_COMMON_OPTS += \
	-D LIBVNCSERVER_INSTALL=ON

CROSS_LFLAG_EXTRA=-lpthread

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

