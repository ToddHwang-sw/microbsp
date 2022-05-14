DIR=qt5
FILENM=$(DIR).git
LOC=-b 5.12 https://code.qt.io/qt

SPECDIR=qtbase/mkspecs
QTPLATFORM=linux-aarch64-any-linux-gnu

MULTI=-j$(shell cat /proc/cpuinfo | grep processor | wc -l)

include $(TOPDIR)/scripts/uixenv.mk

MICB_DEPENDS=\
		dbus-1 iconv intl libdrm libxml-2.0 			\
	       	gio-2.0 gthread-2.0    					\
		sndfile json-c						\
		freetype2 fontconfig libpng libpng16 libjpeg libpulse 	\
		x11 xcb xshmfence xcb-present xfixies xcb-dri3 xkbcommon-x11	\
		pygobject 						\
		libpcre2-posix libpcre2-16 libpcre2-32			\
		gstreamer-plugins-base-1.0				\
		gstreamer-gl-wayland-1.0  gstreamer-gl-egl-1.0		\
		libavdevice 						\
		wayland-egl wayland-client wayland-cursor wayland-server\
		libsystemd ncurses liblz4 libcap gl egl                 \
		libevdev mtdev libudev harfbuzz \


## save time ??? 
QT_CFLAGS:=$(CROSS_USER_CFLAGS)
QT_LFLAGS:=$(CROSS_USER_LFLAGS)

# library 
EXTRA_LIBDIRS:=$(subst -L,,$(QT_LFLAGS))
EXTRA_LIBDIRS:=$(shell echo $(EXTRA_LIBDIRS) | sed -e "s/\(-[a-zA-Z0-9\-_\.,]*\)//g")

## include 
EXTRA_INCDIRS:=$(subst -I,,$(QT_CFLAGS))
EXTRA_INCDIRS:=$(shell echo $(EXTRA_INCDIRS) | sed -e "s/\(-[a-zA-Z0-9\-_\.,]*\)//g")

## TO BE FIXED !! 
QT_LFLAGS += -lglapi

python_req:
	@[ -f /usr/bin/python ] || ( \
		echo "Python executor '/usr/bin/python' is required !! " ; exit 1 )

prepare: python_req
	@[ -d $(BUILDDIR)/$(DIR) ] || ( \
		mkdir -p $(BUILDDIR)/$(DIR) ; \
		[ ! -d $(MICBSRC)/$(DIR)/$(SPECDIR)/$(QTPLATFORM) ] || \
			\rm -rf $(MICBSRC)/$(DIR)/$(SPECDIR)/$(QTPLATFORM) \
		)
	@[ -d $(MICBSRC)/$(DIR) ] || ( \
		$(UNCOMPRESS) $(LOC)/$(DIR).git; 			\
		pushd $(MICBSRC)/$(DIR); 				\
		perl init-repository; 					\
		popd ; 							\
		$(MICB_PATCH) )
	@[ -f $(MICBSRC)/$(DIR)/$(SPECDIR)/$(QTPLATFORM)/qmake.conf ] || (	\
		[ -d $(MICBSRC)/$(DIR)/$(SPECDIR)/$(QTPLATFORM) ] || 		\
			mkdir -p $(MICBSRC)/$(DIR)/$(SPECDIR)/$(QTPLATFORM) ;	\
		touch $(MICBSRC)/$(DIR)/$(SPECDIR)/$(QTPLATFORM)/qmake.conf ; 	\
		cd $(MICBSRC)/$(DIR)/$(SPECDIR);				\
		echo ""                                                         >  $(QTPLATFORM)/qmake.conf ; \
		echo "MAKEFILE_GENERATOR = UNIX"                               >>  $(QTPLATFORM)/qmake.conf ; \
		echo "CONFIG             += incremental"                       >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_INCREMENTAL_STYLE = sublib"                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "include(../common/linux.conf)"                           >>  $(QTPLATFORM)/qmake.conf ; \
		echo "include(../common/gcc-base-unix.conf)"                   >>  $(QTPLATFORM)/qmake.conf ; \
		echo "include(../common/g++-unix.conf)"                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_CC      = $(CC)  $(CROSS_COMP_FLAGS) $(QT_CFLAGS)"    >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_CXX     = $(CXX) $(CROSS_COMP_FLAGS) $(QT_CFLAGS)"    >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_LINK    = $(CXX)     $(CROSS_COMP_FLAGS) $(QT_CFLAGS) $(QT_LFLAGS) $(CROSS_LFLAG_EXTRA)"  >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_LINK_SHLIB  = $(CXX) $(CROSS_COMP_FLAGS) $(QT_CFLAGS) $(QT_LFLAGS) $(CROSS_LFLAG_EXTRA)"  >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_AR      = $(AR) cruv"                              >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_OBJCOPY = $(OBJCOPY)"                              >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_NM      = $(CROSS_COMP_PREFIX)nm -P"               >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_STRIP   = $(STRIP)"                                >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_WAYLAND_SCANNER = $(WAYLAND_BINARIES)/wayland-scanner "  >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "PKG_CONFIG_EXECUTABLE = pkg-config "                     >>  $(QTPLATFORM)/qmake.conf ; \
		echo "PKG_CONFIG_LIBDIR = $(MICB_PKGCONFIG_PATH)"              >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
                echo "EXTRA_LIBDIR        = $(EXTRA_LIBDIRS)"                  >>  $(QTPLATFORM)/qmake.conf ; \
                echo "EXTRA_INCLUDEPATH   = $(EXTRA_INCDIRS)"                  >>  $(QTPLATFORM)/qmake.conf ; \
                echo "QMAKE_INCDIR_OPENGL = $(EXTRA_INCDIRS) "                 >>  $(QTPLATFORM)/qmake.conf ; \
                echo "QMAKE_LIBDIR_OPENGL = $(EXTRA_LIBDIRS) "                 >>  $(QTPLATFORM)/qmake.conf ; \
                echo "QMAKE_INCDIR_ES2    = $(EXTRA_INCDIRS) "                 >>  $(QTPLATFORM)/qmake.conf ; \
                echo "QMAKE_LIBDIR_ES2    = $(EXTRA_LIBDIRS) "                 >>  $(QTPLATFORM)/qmake.conf ; \
                echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "load(qt_config)"                                         >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qplatformdefs.h ; \
		echo "#include \"../linux-g++/qplatformdefs.h\""               >>  $(QTPLATFORM)/qplatformdefs.h ; \
		echo ""                                                        >>  $(QTPLATFORM)/qplatformdefs.h ; \
	) 
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR) ; 			\
		../../../$(MICBSRC)/$(DIR)/configure		\
			-v					\
			-opensource 				\
			-confirm-license			\
			-nomake tests				\
			-opengl es2				\
			-xplatform $(QTPLATFORM) 		\
		)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || (                                                 \
		cd $(BUILDDIR)/$(DIR) ;                                                   \
		make $(MULTI) V=1 )

install:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || (                                                 \
		cd $(BUILDDIR)/$(DIR) ;                                                   \
		make $(MULTI) V=1 INSTALL_ROOT=$(destination) $@ )

clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || \rm -rf $(BUILDDIR)/$(DIR)
	@[ ! -d $(MICBSRC)/$(DIR)/$(SPECDIR)/$(QTPLATFORM) ] || \
			\rm -rf $(MICBSRC)/$(DIR)/$(SPECDIR)/$(QTPLATFORM)
