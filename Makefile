## current directory
export TOPDIR=$(shell pwd)

## by default ubuntu platform 
export TBOARD=unknown

# boards selection
export BDDIR=$(TOPDIR)/boards/$(TBOARD)

# Host system name such as "x86_64-linux" or "i686-linux"
export HOSTSYSTEM=$(shell echo `uname -m`-`uname -s` | awk '{print tolower($$0)}')

# Patch file name 
export DEV_PATCH_FILE=patch.develop

# board specific environment 
include $(BDDIR)/env.mk

#
# "vm" mode needs external USB disk/image file keeping "image.ext4" of /board/vm/.
# USB device node/file image  should be passed to boards/vm/Makefile . 
#
## export EXT4HDD=/dev/sdd1
## export EXT4CFG=/dev/sdd2
export EXT4HDD=$(BDDIR)/$(EXTDISKNM)
export EXT4CFG=$(BDDIR)/$(CFGDISKNM)

# architecture environment 
##
##  Applicatin build directory name
export BUILDDIR=build/$(_ARCH_)

##
## Toolchain Build output file 
export TOOLCHAIN_BUILDOUT=$(TOPDIR)/gnu/toolchain/build_toolchain_log.$(_ARCH_)

##
## Build output file 
export BUILDOUT=$(BUILDDIR)/build.log

define SETUP_BUILDOUT
	[ -d $(1)/$(BUILDDIR) ] || mkdir -p $(1)/$(BUILDDIR)  && \
	[ -f $(1)/$(BUILDOUT) ] || touch $(1)/$(BUILDOUT) 
endef

##
## Build lock folder
##
export BUILD_FOLDER=/var/tmp/microbsp
export BUILD_LOCK_FOLDER=$(BUILD_FOLDER)/lock

##
## exclusive execution of command
##
define PERFORM_EXCLUSIVELY
	$(eval DEPTH  := `echo $$2 | sed "s/\//-/g"`)       \
	$(eval LOCKFN := $(BUILD_LOCK_FOLDER)/microbsp-$1-) \
	[ -f $(LOCKFN)$(DEPTH).$(3) ] || touch $(LOCKFN)$(DEPTH).$(3)  && \
	flock $(LOCKFN)$(DEPTH).$(3) $(4)
endef

## ISO image name...
export IMAGENAME=output.iso

## install dir
INSTSUBFN=disk
XBASEDIR=$(BDDIR)/_base
FINDIR=$(BDDIR)/__tempd
INSTALLDIR:=$(XBASEDIR)/$(INSTSUBFN)
ISODIR=$(BDDIR)/iso
export XBASEDIR
export INSTALLDIR
export ISODIR
export FINDIR
export INSTSUBFN

## Toolchain root ..
export TOOLCHAIN_ROOT=$(TOPDIR)/gnu/toolchain/$(_ARCH_)

## Kernel folder ...
export KERNDIR=$(BDDIR)/kernel/build/$(_ARCH_)

export USRNODE=/usr

## final stage dir 
##
STAGEDIR:=$(BDDIR)/_stagedir
EXTINSTDIR:=$(STAGEDIR)/usr$(USRNODE)
export STAGEDIR
export EXTINSTDIR
TEMPFN=/var/tmp/____temporary_Merge_File______

##
## /uix compiled folders...
UIXINSTDIR:=$(BDDIR)/_uidir$(USRNODE)
export UIXINSTDIR

##
## for processing Library
export LIBPATHFILE=libs.info

##
## This is very important list of path to search *.pc file.
##
## [INSTALLDIR | EXTINSTDIR | UIXINSTDR ] + [ /lib, /lib64, /usr/lib .. ] =
##    Final Paths to look for *.pc file
##
export LIBSSUBDIR=\
	/lib  /lib64          \
	/usr/lib   /usr/lib64 \
	/local/lib  /local/lib64 /local/share

CLEAN_LIBLA=\
	for libdir in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do            \
		for subdir in $(LIBSSUBDIR) ; do                                    \
				[ ! -d $$libdir$$subdir ] ||                                \
				find $$libdir$$subdir -name "*.la" -exec \rm -rf {} \; ;    \
		done ;                                                              \
	done

	
##
## CFLAGS / LFLAGS - for compilation 
##
export INCFLAGS_NAME=$(BUILDDIR)/flags.incs
export LIBFLAGS_NAME=$(BUILDDIR)/flags.libs

##
## GNU toolchains  - component information
##
## 09/30/2022 - binutils/gcc/glibc are updated. 
##
## BINUTILS : 2.32   -->  2.38
##      GCC : 8.3.0  -->  11.2.0
##    GLIBC : 2.30   -->  2.36
##
## export BINUTILS=binutils-2.32
## export GCC=gcc-8.3.0
## export GLIBC=glibc-2.30
##
export BINUTILS=binutils-2.38
export GCC=gcc-11.2.0
export GLIBC_VER=2.36
export GLIBC=glibc-$(GLIBC_VER)

##
## Essential applications 
##
export BASH=bash-5.1.8
export BUSYBOX=busybox-1.35.0

##
## Libraries needed by Bash 
export BASH_PRG=$(TOPDIR)/apps/bash/$(BUILDDIR)/$(BASH)/bash
export BASH_NEEDS_LIBS=$(shell if [ -f $(BASH_PRG) ] ; then ( readelf -a $(BASH_PRG) | grep NEEDED | awk '{print $$5}' | sed -e 's/\[//g' -e 's/\]//g' ) ; fi)

##
## Libraries needed by Busybox utilities 
export BUSY_PRG=$(TOPDIR)/apps/busybox/$(BUILDDIR)/$(BUSYBOX)/busybox
export BUSY_NEEDS_LIBS =$(shell if [ -f $(BUSY_PRG) ] ; then ( readelf -a $(BUSY_PRG) | grep NEEDED | awk '{print $$5}' | sed -e 's/\[//g' -e 's/\]//g' ) ; fi)

## 
## Loader 
export BUSY_NEEDS_INTER=$(shell if [ -f $(BUSY_PRG) ] ; then ( readelf -a $(BUSY_PRG) | grep interpreter | awk '{print $$4}' | sed -e 's/]//g' ) ; fi)

export BOOTSTRAP_LIBS=$(sort $(BUSY_NEEDS_LIBS) $(BASH_NEEDS_LIBS))
export BOOTSTRAP_LDRS=$(BUSY_NEEDS_INTER)

##
## Loader installed by ...
export INSTALLED_LDRS=$(FINDIR)/$(INSTSUBFN)/lib/$(notdir $(BOOTSTRAP_LDRS))

# architecture environment 
include arch/$(_ARCH_)/env.mk

## Toolchain options...
CHECK_TOOLCHAIN=`command -v $(CC) | wc -l`

## Libraries
LIBDIR=\
	zlib \
	pcre \
	lzo \
	liblzma \
	bzip2 \
	lz4 \
	zstd \
	gmp \
	mpfr \
	mpc  \
	isl \
	libffi \
	libnl \
	ncurses \
	readline \
	libcap \
	util-linux \
	e2fsprogs \
	libparted \
	openssl \
	libssh2 \
	nettle \
	flex \
	lsjson \
	pcre2 \
	libxml  \
	expat \
	libpam \
	libpcap \
	libev \
	libtool \
	libattr
SUBDIR+=$(LIBDIR)

## Applications 
APPDIR=\
	busybox \
	bash \
	vim \
	glibc
SUBDIR+=$(APPDIR)

# External disk 
EXTDIR=\
	btrfs \
	binutils \
	libbind \
	fdisk \
	iperf \
	ncftp \
	ntpclient \
	sqlite \
	iconv \
	autoconf \
	automake \
	proxy-libintl \
	m4 \
	make \
	gettext \
	glib \
	pkg-config \
	module-tools \
	procps \
	strace \
	openssh \
	gcc \
	gdb \
	gdbm \
	perl \
	python \
	pciaccess \
	drm    \
	gobject-introspection \
	libidn \
	gnutls \
	curl \
	git \
	xconfig \
	http/fastcgi \
	http/lighttpd \
	tcpdump \
	json \
	jq \
	texinfo \
	help2man  \
	mtdev \
	lpps
SUBDIR+=$(EXTDIR)

## required library application folders
SUBDIR+=$(extra_SUBDIR)

##
##
## UI-related Materials - excluded from SUBDIR set. 
##
##
UIDIR=\
	libjpeg                \
	libpng	               \
	freetype2              \
	fontconfig             \
	libgpg-error           \
	libgcrypt              \
	libdbus	               \
	sndfile	               \
	X11/libxslt            \
	X11/xorg-macros        \
	X11/xorg-doctools      \
	X11/xorg-x11proto      \
	X11/xorg-xtrans        \
	X11/xorg-kbproto       \
	X11/libXau             \
	X11/xorg-xcbproto      \
	X11/xorg-libxcb        \
	X11/xorg-proto         \
	X11/libX11             \
	X11/libICE             \
	X11/xorg-libsm         \
	X11/libXt              \
	X11/libXext            \
	X11/libXmu             \
	X11/libXpm             \
	X11/libXaw             \
	X11/libXrender         \
	X11/libXfixes          \
	X11/libXinput          \
	X11/libXdamage         \
	X11/libXcursor         \
	X11/xorg-libxtst       \
	X11/xshmfence          \
	X11/libXxf86vm         \
	X11/libXrandr          \
	libseccomp             \
	systemd                \
	libevdev               \
	wayland-host           \
	wayland                \
	wayland-protocols      \
	xkbcommon              \
	pixman                 \
	mesa                   \
	cairo                  \
	fribidi                \
	harfbuzz               \
	gdk-pixbuf             \
	pango                  \
	epoxy                  \
	graphene               \
	gtk                    \
	libinput               \
	libva                  \
	weston                 \
	libalsa                \
	libSDL                 \
	ffmpeg                 \
	pygobject              \
	gstreamer/base         \
	gstreamer/plugin-base  \
	gstreamer/plugin-good  \
	gstreamer/plugin-bad   \
	gstreamer/plugin-ugly  \
	gstreamer/libav        \
	gstreamer/python       \
	gstreamer/vaapi        \
	pulseaudio             \
	microwindows	       \
	qt
SUBDIR+=$(UIDIR)

# Component folders - apps, libs, exts, uix
COMPDIR=libs apps exts uix

## YOU CANNOT CHANGE THIS !! .. {apps,libs,uix}/<..>/source
export MICBSRC=source

##
## NOTICE)
##
## $(BDDIR)/rootfs/../inittab should have the definition of console in use and booting login/shellin the first line. 
##
CONSOLECMD=`cat $(BDDIR)/rootfs/etc/inittab | head -n 1`

## made feom WSL2 compilation 
installcomps:  
	@echo ""
	@echo "Installing required SW sets..."
	@echo ""
	@echo ""
	@echo "These menu was tested and validated in Ubuntu LTS 20.04,"
	@echo "but not yet perfectly probed for LTS 22.04"
	@echo "User may get errors in ..."
	@echo "  uix/systemd, "
	@echo "  uix/mesa,    " 
	@echo ""
	@echo ""
	@sudo apt --fix-broken install 
	@sudo apt update 
	@sudo apt install \
		build-essential     cmake                 automake        autoconf          \
		m4                  autopoint             unzip           p7zip-full        \
		autoconf-archive    autogen               texlive         flex              \
		bison               gettext               net-tools       texinfo           \
		libssl-dev          libncurses-dev        libmpc-dev      libreadline-dev   \
		uuid-dev            zlib1g-dev            liblzo2-dev     libelf-dev 	    \
		libtool-bin         pkg-config            chrpath         diffstat          \
		gcc-multilib        g++-multilib          gawk            python3           \
		python3-pip         python3-setuptools    python3-wheel   ninja-build       \
	  	meson               qemu-system-x86       libxml2-dev     libffi-dev        \
	  	help2man            valgrind              gperf           libglib2.0-dev-bin\
	  	ragel               gengetopt             python3-venv    python3-jinja2    \
		gtk-doc-tools       uml-utilities         bridge-utils 
	@sudo apt --no-install-recommends install \
		xsltproc 			xmlto 			      fop 
	@sudo pip install pkgconfig mako Jinja2
	@[ ! -f /usr/local/pip3.8 ] || /usr/local/bin/pip3.8 install mako
	@pip3 install package_name setuptools --user

checkfirst:
	@[ "$(CHECK_TOOLCHAIN)" = "0" ] || ( \
		[ -f $(TOOLCHAIN_ROOT)/bin/$(CC) ] || ( \
			echo ""; \
			echo "Two more cross compilers $(CC) have been installed in the system. Please check. "; \
			echo ""; \
			exit 1 \
		) )
	@[ "$(CHECK_TOOLCHAIN)" != "0" ] || ( \
		echo ""; \
		echo "$(CC) seems not to be ready" ; \
		if [ ! -f $(TOOLCHAIN_ROOT)/bin/$(CC) ]; then \
			echo "Please run \"make toolchain\" first. It will take long time. " ; \
		else \
			echo "Please run \"export PATH=\$$PATH:$(TOOLCHAIN_ROOT)/bin\" simply." ; \
		fi; \
		echo "" ; \
		exit 1 )
	@if [ ! -d $(INSTALLDIR) ]; then \
		mkdir -p $(INSTALLDIR);                                     \
		mkdir -p $(INSTALLDIR)/etc;                                 \
		mkdir -p $(INSTALLDIR)/dev;                                 \
		mkdir -p $(INSTALLDIR)/proc;                                \
		mkdir -p $(INSTALLDIR)/home;                                \
		mkdir -p $(INSTALLDIR)/home/root;                           \
		mkdir -p $(INSTALLDIR)/root;                                \
		mkdir -p $(INSTALLDIR)/var;                                 \
		mkdir -p $(INSTALLDIR)/sys;                                 \
		mkdir -p $(INSTALLDIR)/mnt;                                 \
		mkdir -p $(INSTALLDIR)/bin;                                 \
		mkdir -p $(INSTALLDIR)/sbin;                                \
		mkdir -p $(INSTALLDIR)/usr/bin;                             \
		mkdir -p $(INSTALLDIR)/usr/sbin;                            \
		mkdir -p $(INSTALLDIR)/include;                             \
		mkdir -p $(INSTALLDIR)/lib;                                 \
		mkdir -p $(INSTALLDIR)/boot;                                \
		mkdir -p $(INSTALLDIR)/config;                              \
		ln -s /var/tmp/tmp  $(INSTALLDIR)/tmp;                      \
		ln -s /var/tmp/fstab $(INSTALLDIR)/etc/fstab;               \
		ln -s /var/tmp/mtab  $(INSTALLDIR)/etc/mtab;                \
		ln -s /var/tmp/group $(INSTALLDIR)/etc/group;               \
		ln -s /var/tmp/issue $(INSTALLDIR)/etc/issue;               \
		ln -s /var/tmp/passwd  $(INSTALLDIR)/etc/passwd;            \
		ln -s /var/tmp/shadow  $(INSTALLDIR)/etc/shadow;            \
		ln -s /var/tmp/resolv.conf $(INSTALLDIR)/etc/resolv.conf;   \
	fi
	@[ -d $(STAGEDIR) ] || ( \
		mkdir -p $(STAGEDIR); \
		mkdir -p $(STAGEDIR)/up;   \
		mkdir -p $(STAGEDIR)/work; \
		mkdir -p $(STAGEDIR)/usr/root; \
		mkdir -p $(STAGEDIR)/usr/home; \
		mkdir -p $(EXTINSTDIR); \
	)
	@[ -d $(UIXINSTDIR) ] || ( \
		mkdir -p $(UIXINSTDIR); \
		mkdir -p $(UIXINSTDIR)/include; \
		mkdir -p $(UIXINSTDIR)/lib; \
		mkdir -p $(UIXINSTDIR)/share ; \
		mkdir -p $(UIXINSTDIR)/local/include; \
		mkdir -p $(UIXINSTDIR)/local/lib; \
		mkdir -p $(UIXINSTDIR)/local/share; \
		mkdir -p $(UIXINSTDIR)/local/share/aclocal; \
	)
	@[ ! -f $(BDDIR)/$(LIBPATHFILE) ] || \rm -rf $(BDDIR)/$(LIBPATHFILE)  
	@touch $(BDDIR)/$(LIBPATHFILE) 
	@for libdir in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do	\
		for subdir in $(LIBSSUBDIR) ; do 			\
			echo $$libdir$$subdir >> $(BDDIR)/$(LIBPATHFILE) ; \
			mkdir -p $$libdir$$subdir ;	 		\
		done ;							\
	done
	@[ -d $(BUILD_FOLDER)      ] || mkdir -p $(BUILD_FOLDER)
	@[ -d $(BUILD_LOCK_FOLDER) ] || mkdir -p $(BUILD_LOCK_FOLDER)

##
## libs
##
lib: checkfirst
	@cd libs; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] || ( \
				$(call SETUP_BUILDOUT,$$dir)                                                   && \
				$(call PERFORM_EXCLUSIVELY,lib,$$dir,prepare,\
						make -C $$dir destination=$(INSTALLDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT)\
						) && \
				make -C $$dir destination=$(INSTALLDIR) all      2>&1  | tee -a $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) install  2>&1  | tee -a $$dir/$(BUILDOUT) && \
				$(CLEAN_LIBLA) ) \
		done

##
## lib_prepare , lib_all , lib_install
##
lib_%: checkfirst
	@cd libs; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] ||  (     \
				$(call SETUP_BUILDOUT,$$dir)                                                   && \
				if [ "$(subst lib_,,$@)" = "prepare" ]; then \
					$(call PERFORM_EXCLUSIVELY,lib,$$dir,prepare,\
						make -C $$dir destination=$(INSTALLDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT)\
						) ; \
				else  \
					make -C $$dir destination=$(INSTALLDIR) $(subst lib_,,$@) 2>&1 | tee $$dir/$(BUILDOUT) ; \
				fi && \
				[ "$(subst lib_,,$@)" != "install" ] || $(CLEAN_LIBLA)  \
			) ;	\
		done

##
## apps
##
app: checkfirst
	@cd apps; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] || ( \
				$(call SETUP_BUILDOUT,$$dir)                                                   && \
				$(call PERFORM_EXCLUSIVELY,app,$$dir,prepare,\
					make -C $$dir destination=$(INSTALLDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT)\
					) && \
				make -C $$dir destination=$(INSTALLDIR) all      2>&1  | tee -a $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) install  2>&1  | tee -a $$dir/$(BUILDOUT) && \
				$(CLEAN_LIBLA) ) \
		done

##
## app_prepare , app_all , app_install
##
app_%: checkfirst
	@cd apps; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] ||  (     \
				$(call SETUP_BUILDOUT,$$dir)                                                   && \
				if [ "$(subst lib_,,$@)" = "prepare" ]; then \
					$(call PERFORM_EXCLUSIVELY,app,$$dir,prepare,\
						make -C $$dir destination=$(INSTALLDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT)\
						) ; \
				else  \
					make -C $$dir destination=$(INSTALLDIR) $(subst app_,,$@) 2>&1 | tee $$dir/$(BUILDOUT) ; \
				fi && \
				[ "$(subst app_,,$@)" != "install" ] || $(CLEAN_LIBLA)  \
			) ;	\
		done

##
## exts
##
ext: checkfirst
	@cd exts; \
		for dir in $(SUBDIR); do    \
			[ ! -d $$dir ] || ( \
				$(call SETUP_BUILDOUT,$$dir)                                                   && \
				$(call PERFORM_EXCLUSIVELY,ext,$$dir,prepare,\
					make -C $$dir destination=$(EXTINSTDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT)\
					) && \
				make -C $$dir destination=$(EXTINSTDIR) all      2>&1  | tee -a $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(EXTINSTDIR) install  2>&1  | tee -a $$dir/$(BUILDOUT) && \
				$(CLEAN_LIBLA) ) \
		done

##
## ext_prepare , ext_all , ext_install
##
ext_%: checkfirst
	@cd exts; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] ||  (     \
				$(call SETUP_BUILDOUT,$$dir)                                                   && \
				if [ "$(subst lib_,,$@)" = "prepare" ]; then \
					$(call PERFORM_EXCLUSIVELY,ext,$$dir,prepare,\
						make -C $$dir destination=$(EXTINSTDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT)\
						) ; \
				else  \
					make -C $$dir destination=$(EXTINSTDIR) $(subst ext_,,$@) 2>&1 | tee $$dir/$(BUILDOUT) ; \
				fi && \
				[ "$(subst ext_,,$@)" != "install" ] || $(CLEAN_LIBLA)  \
			) ;	\
		done

##
##  ui
##
ui: checkfirst llvm_okay
	@cd uix; \
		for dir in $(SUBDIR); do    \
			[ ! -d $$dir ] || ( \
				$(call SETUP_BUILDOUT,$$dir)                                                   && \
				$(call PERFORM_EXCLUSIVELY,ui,$$dir,prepare,\
					make -C $$dir destination=$(UIXNSTDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT)\
					) && \
				make -C $$dir destination=$(UIXINSTDIR) all      2>&1  | tee -a $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(UIXINSTDIR) install  2>&1  | tee -a $$dir/$(BUILDOUT) && \
				$(CLEAN_LIBLA) ) \
		done
	
##
## ui_prepare , ui_all , ui_install
##
ui_%: checkfirst llvm_okay
	@cd uix; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] ||  (     \
				$(call SETUP_BUILDOUT,$$dir)                                                   && \
				if [ "$(subst lib_,,$@)" = "prepare" ]; then \
					$(call PERFORM_EXCLUSIVELY,ui,$$dir,prepare,\
						make -C $$dir destination=$(UIXINSTDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT)\
						) ; \
				else  \
					make -C $$dir destination=$(UIXINSTDIR) $(subst ui_,,$@) 2>&1 | tee $$dir/$(BUILDOUT) ; \
				fi && \
				[ "$(subst ui_,,$@)" != "install" ] || $(CLEAN_LIBLA)  \
			) ;	\
		done
	
##
## projects
##
proj: checkfirst 
	@make -C $(BDDIR)/projects destination=$(EXTINSTDIR) prepare all install
	@$(CLEAN_LIBLA)

proj_%: checkfirst 
	@make -C $(BDDIR)/projects destination=$(EXTINSTDIR) $(subst proj_,,$@) 
	@$(CLEAN_LIBLA)


##
## LLVM compiler -- reuqired by UI sets...
##
llvm_okay:
	@[ -f $(TOOLCHAIN_ROOT)/bin/$(PLATFORM)-gcc ] || ( \
		echo ""; \
		echo "Base cross-compiler should be installed " ; \
		echo ""; \
		exit 1 \
		)	

llvm: llvm_okay
	@make -C gnu/llvm prepare all install
	@$(CLEAN_LIBLA)

llvm_%: llvm_okay
	@make -C gnu/llvm $(subst llvm_,,$@)
	@$(CLEAN_LIBLA)


##
## clean
## distclean 
##
clean: checkfirst
	@for cat in $(COMPDIR); do                 \
		cd $$cat ;                             \
		for dir in $(SUBDIR); do               \
			[ ! -d $$dir ] || make -C $$dir clean || exit 1; \
		done ;                                 \
		cd .. ;                                \
	done

distclean:
	@for cat in $(COMPDIR); do                 \
		cd $$cat ;                             \
		for dir in $(SUBDIR); do               \
			[ ! -d $$dir ] || (                \
				[ ! -d $$dir/$(BUILDDIR) ] || \rm -rf $$dir/$(BUILDDIR)        ; \
				[ ! -f $$dir/$(BUILDOUT) ] || \rm -f $$dir/$(BUILDOUT)         ; \
				[ ! -f $$dir/$(LIBFLAGS_NAME) ] || rm -f $$dir/$(LIBFLAGS_NAME); \
				[ ! -f $$dir/$(INCFLAGS_NAME) ] || rm -f $$dir/$(INCFLAGS_NAME); \
			)                                  \
		done ;                                 \
		cd .. ;                                \
	done

wipeout:
	@for cdir in $(COMPDIR); do                 						        \
		for dir in $(SUBDIR); do               						            \
			if [ -f $$cdir/$$dir/Makefile ] &&  					            \
					[ -d $$cdir/$$dir/$(MICBSRC) ]; then 			            \
				XDIR=`cat $$cdir/$$dir/Makefile | grep 'DIR=' | head -n 1`; 	\
				XDIR=$${XDIR#DIR=} ; 						                    \
				if [ ! -f $$cdir/$$dir/$(MICBSRC)/.nocleanup ]; then		    \
					\rm -rf $$cdir/$$dir/$(MICBSRC)/* ;			                \
					echo "Cleaning in $$cdir/$$dir/$(MICBSRC)/*" ;		        \
				fi;								                                \
			fi ;									                            \
		done ;                                						            \
	done

##
## extdisk
## extdisk_clean
##
extdisk:
	@cd gnu; make -f Makefile hdrpath=$(EXTINSTDIR) setup_headers
	@$(TOPDIR)/scripts/setupdisk.sh build $(STAGEDIR) $(EXTDISK) $(BDDIR)/$(EXTDISKNM) $(EXTDISKBLKS)
	@[ -z $(CFGDISKNM) ] || sudo dd if=/dev/zero of=$(EXT4CFG) bs=1M count=$(CFGDISKBLKS)
	@[ -z $(CFGDISKNM) ] || sudo mkfs.ext4 -j -F -L $(CFGVOLNM) $(EXT4CFG)


extdisk_clean:
	@$(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(EXTDISKNM)
	@[ -z $(CFGDISKNM) ] || ( [ ! -f $(BDDIR)/$(CFGDISKNM) ] || \rm -rf $(BDDIR)/$(CFGDISKNM) )
			

##
## uidisk
## uidisk_clean
##
uidisk:
	@[ "$(UIDISK)" = "" ] || \
		$(TOPDIR)/scripts/setupdisk.sh build $(UIXINSTDIR) $(UIDISK) $(BDDIR)/$(UIDISKNM) $(UIDISKBLKS)

uidisk_clean:
	@[ "$(UIDISK)" = "" ] || \
		$(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(UIDISKNM)
			

##
## BUILDUP_ROOTFS --> Please refer to boards/rpi3/env.mk 
##
run_bootstrap: checkfirst
	@make -C apps/busybox         destination=$(XBASEDIR) -f Makefile.bootstrap prepare all install
	@make -C apps/bash            destination=$(XBASEDIR) install
	@make -C apps/tools/overlayfs destination=$(XBASEDIR) prepare all install
	@$(shell $(BUILDUP_ROOTFS)) > /dev/null
	@[ ! -f $(XBASEDIR)/etc/init.d/rcS ]         || chmod ugo+x $(XBASEDIR)/etc/init.d/rcS
	@[ ! -f $(XBASEDIR)/etc/init.d/rc.shutdown ] || chmod ugo+x $(XBASEDIR)/etc/init.d/rc.shutdown

boot_%:
	@echo ""
	@echo "Bootloader configuration/cleanup"
	@echo ""
	@make -C $(BDDIR)/boot isodir=$(ISODIR) $(subst boot_,,$@)

kernel_%:
	@echo ""
	@echo "Kernel configuration/cleanup"
	@echo ""
	@make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) $(subst kernel_,,$@)
	@if [ "$(subst kernel_,,$@)" = "clean" ] ; then \
		echo ""	                     ; \
		echo "Cleaning image..."     ; \
		echo ""                      ; \
	       	make -C $(BDDIR) clean       ; \
	fi

modules_%:
	@if [ "$(subst mod_,,$@)" = "install" ] ; then \
		make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) install ;              \
		make -C $(BDDIR)        destination=$(INSTALLDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) install ; \
	fi

board: run_bootstrap
	@echo ""
	@echo "Preparing board"
	@echo ""
	@make -C $(BDDIR) destination=$(INSTALLDIR) isodir=$(ISODIR) prepare
	@echo ""
	@echo "Building kernel..."
	@echo ""
	@make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) dnfw prepare all
	@echo ""
	@echo "External modules...."
	@echo ""
	@make -C $(BDDIR)/kernel KERNDIR=$(KERNDIR) destination=$(INSTALLDIR) extmod
	@echo ""
	@echo "Copying static rootfs...."
	@echo ""
	@cp -rf $(BDDIR)/rootfs/* $(INSTALLDIR)/ 
	@echo ""
	@echo "Merging $(INSTALLDIR) and GLIBC ...."
	@echo ""
	@[ ! -d $(FINDIR) ] || sudo \rm -rf $(FINDIR)
	@mkdir -p $(FINDIR)
	@cd $(XBASEDIR) ; \
		sudo find . -depth -print | sudo cpio -pdm $(FINDIR) ; \
		cd $(TOPDIR) ; \
		make -C apps/glibc destination=$(FINDIR)/$(INSTSUBFN) install_glibc
	@echo ""
	@echo "Setting for copying $(BOOTSTAP_LIBS) and $(BOOTSTRAP_LDRS) .. "
	@echo ""
	@cd $(FINDIR)/$(INSTSUBFN)/lib ; \
		for fn in $(BOOTSTRAP_LIBS); do [ ! -f $$fn ] || cp -f $$fn ../../lib/ ; done
	@cd $(FINDIR); \
		[ -d $(dir $(BOOTSRTAP_LDRS)) ] || mkdir -p $(dir $(BOOTSTRAP_LDRS))
	@cd $(FINDIR)/$(INSTSUBFN); \
		[ -d $(dir $(BOOTSRTAP_LDRS)) ] || mkdir -p $(dir $(BOOTSTRAP_LDRS))
	@echo ""
	@echo "Copying $(notdir $(BOOTSTRAP_LDRS)) to $(FINDIR)/$(INSTSUBFN)/lib64"
	@echo ""
	@[ -f $(FINDIR)/$(INSTSUBFN)/lib64/$(notdir $(BOOTSTRAP_LDRS)) ] || cp -rf  $(INSTALLED_LDRS)  $(FINDIR)/$(INSTSUBFN)/lib64
	@echo ""
	@echo "Copying $(notdir $(BOOTSTRAP_LDRS)) to $(FINDIR)/lib64"
	@echo ""
	@[ -f $(FINDIR)/lib64/$(notdir $(BOOTSTRAP_LDRS)) ]              || cp -rf  $(INSTALLED_LDRS)  $(FINDIR)/lib64
	@echo ""
	@echo "Copying $(notdir $(BOOTSTRAP_LDRS)) to $(FINDIR)/lib"
	@echo ""
	@[ -f $(FINDIR)/lib/$(notdir $(BOOTSTRAP_LDRS)) ]                || cp -rf  $(INSTALLED_LDRS)  $(FINDIR)/lib

ramdisk:
	@echo ""
	@echo "Compiling kernel and combining squash image... "
	@echo ""
	@make -C $(BDDIR)/kernel destination=$(FINDIR) isodir=$(ISODIR) install
	@echo ""
	@echo "Build up image..."
	@echo ""
	@make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) install

cleanup:
	@[ ! -d $(XBASEDIR)   ] || \rm -rf $(XBASEDIR)
	@[ ! -d $(STAGEDIR)   ] || \rm -rf $(STAGEDIR)
	@[ ! -d $(UIXINSTDIR) ] || \rm -rf $(UIXINSTDIR)
	@[ ! -d $(FINDIR)     ] || \rm -rf $(FINDIR)
	@[ ! -d $(ISODIR)     ] || \rm -rf $(ISODIR)
	@make -C $(BDDIR)        uninstall
	@make -C $(BDDIR)/kernel uninstall
	@cd $(BDDIR); \
		$(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(EXTDISKNM)

toolchain:
	@[ -f $(TOOLCHAIN_BUILDOUT) ] || touch $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "Preparing sources.... "                2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@make -C gnu/sources -f ../Makefile download
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "BINUTILS....."                         2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@[ -d gnu/binutils/build/$(_ARCH_) ] || mkdir -p gnu/binutils/build/$(_ARCH_)
	@cd gnu/binutils/build/$(_ARCH_); \
		make -f ../../../Makefile config_binutils setup_binutils  2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "Kernel Headers....."                   2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@[ -d $(KERNDIR)/$(KERNELVER) ] || \
		make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) prepare 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@cd gnu; \
		make -f Makefile hdrpath=$(TOOLCHAIN_ROOT)/$(PLATFORM) setup_headers 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@[ ! -f $(TOOLCHAIN_ROOT)/$(PLATFORM)/include/asm/.install ] || ( \
		echo ""  ;                     \
		echo "Cleaning up kernel folder" ; \
		echo ""  ;                     \
		\rm -rf $(KERNDIR)/$(KERNELVER) ; )
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GCC Bootstrap ....."                   2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@[ -d gnu/gcc/build/$(_ARCH_) ] || mkdir -p gnu/gcc/build/$(_ARCH_)
	@cd gnu/gcc/build/$(_ARCH_); \
		make -f ../../../Makefile config_gcc setup_gcc          2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GLIBC Bootstrap ....."                 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@[ -d gnu/glibc/build/$(_ARCH_) ] || mkdir -p gnu/glibc/build/$(_ARCH_)
	@cd gnu/glibc/build/$(_ARCH_); \
		make -f ../../../Makefile config_glibc setup_glibc      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GCC Host Compiler....."                2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@cd gnu/gcc/build/$(_ARCH_); \
		make -f ../../../Makefile setup_gcc_2     2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GLIBC Host Library....."               2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@cd gnu/glibc/build/$(_ARCH_); \
		make -f ../../../Makefile setup_glibc_2   2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GCC Extra Libraries....."              2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@cd gnu/gcc/build/$(_ARCH_); \
		make -f ../../../Makefile setup_gcc_3     2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)

#########################################################################################################################
#
#
#  x86_64 VM execution ... TBOARD=vm option only 
#
#  # make TBOARD=vm [vmrun|vmstop] 
#
#
#########################################################################################################################

vm%:
	@[ "$(TBOARD)" = "vm" ] || ( echo "Only \"TBOARD=vm\" option is eligible for QEMU based VM"; exit 1 )
	@if [ "$(subst vm,,$@)" = "run" ]; then           \
		make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) run ;       \
	elif [ "$(subst vm,,$@)" = "stop" ]; then         \
		make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) stop ;      \
	else                                              \
		echo "Only two commands; vmrun top " ;        \
	fi

pkglist:
	@for dir in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do 	\
		for subdir in $(LIBSSUBDIR) ; do 			\
	    		[ ! -d $$dir$$subdir/pkgconfig ] || 		\
				find $$dir$$subdir -name "*.pc" ; 	\
			done ;						\
		done

