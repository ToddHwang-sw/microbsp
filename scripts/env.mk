include $(TOPDIR)/arch/$(_ARCH_)/env.mk

export CROSS_COMP_FLAGS += -include linux/limits.h 

export CROSS_USER_CFLAGS =
export CROSS_USER_LFLAGS =
export CROSS_LFLAG_EXTRA = 

## defined in each of Makefile...
MICB_DEPENDS =

COLLECT_LIBS=\
	$(eval _CDIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))) \
	[ -d $(_CDIR)/$(BUILDDIR) ] || mkdir -p $(_CDIR)/$(BUILDDIR) ; \
	if [ ! -f $(_CDIR)/$(LIBFLAGS_NAME) ]; then \
		for dep in $(MICB_DEPENDS) ; do \
			for grp in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do \
				for dir in $(LIBSSUBDIR) ; do  \
					if [ -f $$grp$$dir/pkgconfig/$$dep.pc ] ; then \
						$(TOPDIR)/scripts/filterlibs.sh \
							$(BDDIR) $$grp$$dir/pkgconfig/$$dep.pc $$grp$$dir ; \
					fi ; \
				done ; \
			done ; \
		done > $(_CDIR)/$(LIBFLAGS_NAME); \
	fi ; \
	cat $(_CDIR)/$(LIBFLAGS_NAME)

BASIC_SYSLIBS=-ldl -lpthread 

export DEPLOYED_LIBFLAGS=\
	$(sort $(shell $(COLLECT_LIBS)) $(BASIC_SYSLIBS))

##
## LFLAG
export CROSS_USER_LFLAGS += $(DEPLOYED_LIBFLAGS) 

COLLECT_INCS = \
	$(eval _CDIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))) \
	[ -d $(_CDIR)/$(BUILDDIR) ] || mkdir -p $(_CDIR)/$(BUILDDIR) ; \
	if [ ! -f $(_CDIR)/$(INCFLAGS_NAME) ]; then \
		for dep in $(MICB_DEPENDS) ; do \
			for grp in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do \
				for dir in $(LIBSSUBDIR) ; do  \
					if [ -f $$grp$$dir/pkgconfig/$$dep.pc ] ; then \
						$(TOPDIR)/scripts/filterincs.sh \
							$(BDDIR) $$grp$$dir/pkgconfig/$$dep.pc $$grp$$dir ; \
					fi ; \
				done ; \
			done ; \
		done > $(_CDIR)/$(INCFLAGS_NAME); \
	fi ; \
	cat $(_CDIR)/$(INCFLAGS_NAME)

export DEPLOYED_INCFLAGS=\
	$(sort $(shell $(COLLECT_INCS)))

##
## CFLAGS 
export CROSS_USER_CFLAGS += $(DEPLOYED_INCFLAGS)

## Basic patch file just for configuration
export MICB_PATCH_FN=patch/patch
export MICB_PATCH=[ ! -f $(MICB_PATCH_FN) ] || (cd $(MICBSRC)/$(DIR) && cat ../../$(MICB_PATCH_FN) | patch -p 1)

##
## Duplicate sources ... 
export MICB_DUP_SOURCES=[ ! -d $(MICBSRC)/$(DIR) ] || ( \
				[ -d $(BUILDDIR)/$(DIR) ] || \rm -rf $(BUILDDIR)/$(DIR) ; \
				cp -rf $(MICBSRC)/$(DIR) $(BUILDDIR) )

## /usr/bin/pkg-config search path 
export MICB_PKGCONFIG_PATH=$(INSTALLDIR)/lib/pkgconfig:$(INSTALLDIR)/usr/lib/pkgconfig:$(INSTALLDIR)/usr/local/lib/pkgconfig

## system architecture option selection 
ifeq ($(_CORE_),aarch64)
	KARCH=arm64
else
	KARCH=$(_CORE_)
endif

## Source decompressor
export UNCOMPRESS=$(TOPDIR)/scripts/setupsrcs.sh $(MICBSRC)/$(DIR)

## platform 
export MICB_CONFIGURE_PLATFORM=$(PLATFORM)

##
##   P R E F I X
##

## prefix= for meson build...
export MICB_MESON_PREFIX=

## prefix= for cmake
export MICB_CMAKE_PREFIX=

## prefix= for cmake
export MICB_CONFIGURE_PREFIX=

## ninja command install option 
##
## DESTDIR=xxxx option argument 
##
export MICB_INSTALL_PLACE = $(destination)

##
##
##  C O N F I G U R E 
##
##
export MICB_CONFIGURE_ACLOCAL_FLAGS="-I$(INSTALLDIR)"

export MICB_CONFIGURE_AUTOCONF_CMD=\
		autoreconf --install -v $(MICB_CONFIGURE_ACLOCAL_FLAGS)

## New AUTOCONF --> DUP_SOURCE is included and configuration happens in $(BUILDDIR)/$(DIR)
export MICB_CONFIGURE_AUTOCONF=\
		[ ! -d $(MICBSRC)/$(DIR) ] || ( \
				$(MICB_DUP_SOURCES);                  \
				cd $(BUILDDIR)/$(DIR);                \
				ACLOCAL_CFLAGS=$(MICB_CONFIGURE_ACLOCAL_FLAGS) $(MICB_CONFIGURE_AUTOCONF_CMD) )
export MICB_CONFIGURE_RUNENV=\
		PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) \
		LIBS="$(CROSS_USER_LFLAGS) $(CROSS_LFLAG_EXTRA)"
export MICB_CONFIGURE_LIBOPTS=--enable-shared --disable-static
export MICB_CONFIGURE_OPTS=$(MICB_CONFIGURE_LIBOPTS)
export MICB_CONFIGURE_PRG=../../../$(MICBSRC)/$(DIR)/configure
export MICB_CONFIGURE_CMD=$(MICB_CONFIGURE_RUNENV) $(MICB_CONFIGURE_PRG) --prefix=$(MICB_CONFIGURE_PREFIX) --program-prefix= --program-transform-name= \
		--target=$(MICB_CONFIGURE_PLATFORM) --build=i686-linux --host=$(MICB_CONFIGURE_PLATFORM) 
export MICB_CONFIGURE_MAKEOPTS= V=1 DESTDIR=$(MICB_INSTALL_PLACE)

##
##
##  M E S O N 
##
##
export MICB_MESON_CROSSBUILD_FN=cross_build.txt

export MICB_MESON_PKG_CONFIG="$(TOPDIR)/scripts/pkg-config"

export MICB_MESON_CROSSBUILD_FILE=\
	( \
	[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)                                                  && \
	echo ""                                                        >  $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "[host_machine]"                                          >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "system = 'linux'"                                        >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "cpu_family = '$(_CORE_)'"                                >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "cpu   = '$(KARCH)'"                                      >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "endian = 'little'"                                       >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo ""                                                        >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "[build_machine]"                                         >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "system = 'linux'"                                        >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "cpu_family = 'x86_64'"                                   >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "cpu   = 'x86_64'"                                        >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "endian = 'little'"                                       >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo ""                                                        >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "[binaries]"                                              >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "c     = '$(CROSS_COMP_PREFIX)gcc'"                       >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "cpp   = '$(CROSS_COMP_PREFIX)g++'"                       >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "ar    = '$(CROSS_COMP_PREFIX)ar'"                        >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "ranlib= '$(CROSS_COMP_PREFIX)ranlib'"                    >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "strip = '$(CROSS_COMP_PREFIX)strip'"                     >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "pkgconfig = '$(MICB_MESON_PKG_CONFIG)'"                  >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo ""                                                        >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "[properties]"                                            >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "needs_exe_wrapper = 'true'"                              >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo ""                                                        >> $(BUILDDIR)/$(MICB_MESON_CROSSBUILD_FN) && \
	echo "Cross build done!!" > /dev/null )

export MICB_MESON_COMMON_OPTS=\
	-D c_args="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)"          				\
	-D cpp_args="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)"        				\
	-D c_link_args="$(CROSS_COMP_FLAGS) $(CROSS_USER_LFLAGS) $(CROSS_LFLAG_EXTRA)"     	\
	-D cpp_link_args="$(CROSS_COMP_FLAGS) $(CROSS_USER_LFLAGS) $(CROSS_LFLAG_EXTRA)"   	\
	-D pkg_config_path="$(MICB_PKGCONFIG_PATH)"  \
	-D prefix=$(MICB_MESON_PREFIX)

export MICB_MESON_RUNENV=

export MICB_MESON_OPTS=$(MICB_MESON_COMMON_OPTS)

## should be run in $(BUILDDIR)/$(DIR)
export MICB_MESON_CMD=$(MICB_MESON_RUNENV) meson . ../../../$(MICBSRC)/$(DIR) --cross-file ../$(MICB_MESON_CROSSBUILD_FN)

## Ninja command
export NINJA_MAKE=DESTDIR=$(MICB_INSTALL_PLACE) ninja -v

##
##
##  C M A K E 
##
##
export MICB_CMAKE_SYSROOT=$(subst install: ,,$(shell $(CROSS_COMP_PREFIX)gcc --print-search-dirs | grep install))

export MICB_CMAKE_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH)

export MICB_CMAKE_COMMON_OPTS=\
	-D CMAKE_BUILD_TYPE="Release"                                        \
	-D CMAKE_INSTALL_PREFIX="$(MICB_CMAKE_PREFIX)"                       \
	-D CMAKE_CROSSCOMPILING=True                                         \
	-D CMAKE_C_COMPILER="$(CROSS_COMP_PREFIX)gcc"                        \
	-D CMAKE_SYSROOT="$(MICB_CMAKE_SYSROOT)"                             \
	-D CMAKE_C_FLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS) $(CROSS_LFLAG_EXTRA)"          \
	-D CMAKE_EXE_LINKER_FLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_LFLAGS) $(CROSS_LFLAG_EXTRA)" 

##
## B O O T S T R A P
##
##   gnulib folder location - libs/gnulib
##
export MICB_BOOTSTRAP_CMD=\
	ACLOCAL_FLAGS=$(MICB_CONFIGURE_ACLOCAL_FLAGS) \
		./bootstrap --no-git --skip-po --copy --gnulib-srcdir=$(TOPDIR)/libs/gnulib/$(MICBSRC)/gnulib
