DIR=libpcap
LOC=https://github.com/the-tcpdump-group

include $(TOPDIR)/scripts/env.mk

MICB_CMAKE_COMMON_OPTS +=                 \
	-D PKG_CONFIG_EXECUTABLE=$(MICB_MESON_PKG_CONFIG) \
	-D LIBNL_LIBRARIES=$(INSTALLDIR) \
	-D DISABLE_DBUS=ON \
	-D BUILD_SHARED_LIBS=ON

MICB_DEPENDS=libnl-3.0 

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_CMAKE_RUNENV) cmake $(MICB_CMAKE_COMMON_OPTS) ../../../$(MICBSRC)/$(DIR) 

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) VERBOSE=1 $@ )

