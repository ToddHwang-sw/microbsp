DIR=qt5
FILENM=$(DIR).git
LOC=-b 5.12 https://code.qt.io/qt

QTBLDDIR=qtbuild
SPECDIR=qtbase/mkspecs
QTPLATFORM=linux-aarch64-any-linux-gnu
BUILDPLATFORM=linux-g++

MULTI=-j$(shell cat /proc/cpuinfo | grep processor | wc -l)

include $(TOPDIR)/scripts/uixenv.mk

CROSS_USER_CFLAGS += \
	-I$(UIXINSTDIR)/include/SDL2

QT_EXTRA_LIBS = \
	-ldl \
	-lgbm -lxkbcommon \
	-lpcre2-posix -lpcre2-8  -lpcre2-16 -lpcre2-32 \
	-lglapi -ldbus-1 \
	-lwayland-cursor -lwayland-server -lwayland-egl -lwayland-client 

CROSS_LFLAG_EXTRA = \
	$(LIBS_BASIC) \
	$(LIBS_UIBASIC) \
	$(LIBS_GL)   \
	$(LIBS_GLIB) \
	$(LIBS_X11)  \
	$(LIBS_GSTREAMER) \
	$(QT_EXTRA_LIBS)

##
## Library paths 
## 
EXTRA_LIBDIRS=$(subst -L,,$(CROSS_USER_LFLAGS))

##
## Include paths 
## 
EXTRA_INCDIRS:=$(CROSS_USER_CFLAGS)
EXTRA_INCDIRS:=$(filter-out -Wno-missing-include-dirs,$(EXTRA_INCDIRS))
EXTRA_INCDIRS:=$(shell echo $(EXTRA_INCDIRS) | sed -e 's/-D.*//g')
EXTRA_INCDIRS:=$(subst -I,,$(EXTRA_INCDIRS))

python_req:
	@[ -f /usr/bin/python ] || ( \
		echo "Python executor '/usr/bin/python' is required !! " ; exit 1 )

prepare: python_req
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(FILENM) ] || ( \
		cd $(BUILDDIR); 					\
		git clone --single-branch $(LOC)/$(FILENM) ; 		\
		cd $(DIR) ; 						\
		perl init-repository ; 					\
		cd .. ; 						\
		mkdir -p $(QTBLDDIR) )
	@[ -f $(BUILDDIR)/$(DIR)/$(SPECDIR)/$(QTPLATFORM)/PATCHED ] || ( 		\
		mkdir -p $(BUILDDIR)/$(DIR)/$(SPECDIR)/$(QTPLATFORM) ;          	\
		cd $(BUILDDIR)/$(DIR)/$(SPECDIR)/../../ ;				\
		cat ../../../patch/patch | patch -p 1 ; 				\
		touch $(SPECDIR)/$(QTPLATFORM)/PATCHED ;				\
		)
	@[ -f $(BUILDDIR)/$(DIR)/$(SPECDIR)/$(QTPLATFORM)/qbase.conf ] || (	       	\
		touch $(BUILDDIR)/$(DIR)/$(SPECDIR)/$(QTPLATFORM)/qmake.conf ; 	\
		cd $(BUILDDIR)/$(DIR)/$(SPECDIR);				\
		echo ""                                                         >  $(QTPLATFORM)/qmake.conf ; \
		echo "MAKEFILE_GENERATOR = UNIX"                               >>  $(QTPLATFORM)/qmake.conf ; \
		echo "CONFIG             += incremental"                       >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_INCREMENTAL_STYLE = sublib"                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "include(../common/linux.conf)"                           >>  $(QTPLATFORM)/qmake.conf ; \
		echo "include(../common/gcc-base-unix.conf)"                   >>  $(QTPLATFORM)/qmake.conf ; \
		echo "include(../common/g++-unix.conf)"                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_CC      = $(CC)  $(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)"    >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_CXX     = $(CXX) $(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)"    >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_LINK    = $(CXX)     $(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS) $(CROSS_USER_LFLAGS) $(CROSS_LFLAG_EXTRA)"  >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_LINK_SHLIB  = $(CXX) $(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS) $(CROSS_USER_LFLAGS) $(CROSS_LFLAG_EXTRA)"  >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_AR      = $(AR) cruv"                              >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_OBJCOPY = $(OBJCOPY)"                              >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_NM      = $(CROSS_COMP_PREFIX)nm -P"               >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_STRIP   = $(STRIP)"                                >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_WAYLAND_SCANNER = $(WAYLAND_BINARIES)/wayland-scanner "  >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "EXTRA_LIBDIR        = $(EXTRA_LIBDIRS)"                  >>  $(QTPLATFORM)/qmake.conf ; \
		echo "EXTRA_INCLUDEPATH   = $(EXTRA_INCDIRS)"                  >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_INCDIR_OPENGL = $(EXTRA_INCDIRS) "                 >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_LIBDIR_OPENGL = $(EXTRA_LIBDIRS) "                 >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_INCDIR_ES2    = $(EXTRA_INCDIRS) "                 >>  $(QTPLATFORM)/qmake.conf ; \
		echo "QMAKE_LIBDIR_ES2    = $(EXTRA_LIBDIRS) "                 >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "PKG_CONFIG_EXECUTABLE = pkg-config "                     >>  $(QTPLATFORM)/qmake.conf ; \
		echo "PKG_CONFIG_LIBDIR = $(MICB_PKGCONFIG_PATH)"              >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qmake.conf ; \
		echo "load(qt_config)"                                         >>  $(QTPLATFORM)/qmake.conf ; \
		echo ""                                                        >>  $(QTPLATFORM)/qplatformdefs.h ; \
		echo "#include \"../linux-g++/qplatformdefs.h\""               >>  $(QTPLATFORM)/qplatformdefs.h ; \
		echo ""                                                        >>  $(QTPLATFORM)/qplatformdefs.h ; \
	) 
	@[ ! -d $(BUILDDIR)/$(QTBLDDIR) ] || ( \
		cd $(BUILDDIR)/$(QTBLDDIR) ; 			\
		../$(DIR)/configure				\
			-v					\
			-opensource 				\
			-confirm-license			\
			-nomake tests				\
			-opengl es2				\
			-xplatform $(QTPLATFORM) 		\
		)

all:
	@[ ! -d $(BUILDDIR)/$(QTBLDDIR) ] || (                                                 \
		cd $(BUILDDIR)/$(QTBLDDIR) ;                                                   \
		make $(MULTI) V=1 )

install:
	@[ ! -d $(BUILDDIR)/$(QTBLDDIR) ] || (                                                 \
		cd $(BUILDDIR)/$(QTBLDDIR) ;                                                   \
		make $(MULTI) V=1 INSTALL_ROOT=$(destination) $@ )

clean:
	@[ ! -d $(BUILDDIR)/$(QTBLDDIR) ] || \rm -rf $(BUILDDIR)/$(QTBLDDIR)
	@[ ! -f $(BUILDDIR)/$(DIR)/$(SPECDIR)/$(QTPLATFORM)/qbase.conf ] || \
			\rm -rf $(BUILDDIR)/$(DIR)/$(SPECDIR)/$(QTPLATFORM)
