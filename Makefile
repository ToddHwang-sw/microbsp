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

# architecture environment 
##
##  Applicatin build directory name
export BUILDDIR=build/$(_ARCH_)

##
## Build output file 
export BUILDOUT=build/output.$(_ARCH_)

## ISO image name...
export IMAGENAME=output.iso

## install dir
INSTSUBFN=disk
XBASEDIR=$(BDDIR)/_install
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

## final stage dir 
##
STAGEDIR:=$(BDDIR)/_stagedir
EXTINSTDIR:=$(STAGEDIR)/usr
export STAGEDIR
export EXTINSTDIR
TEMPFN=/var/tmp/____temporary_Merge_File______

##
## /uix compiled folders...
UIXINSTDIR:=$(BDDIR)/_uidir
export UIXINSTDIR

##
## for processing Library
export LIBPATHFILE=libs.info

export LIBSSUBDIR=\
	/lib /lib64 \
	/usr/lib  /usr/lib64 \
	/usr/local/lib  /usr/local/lib64 

CLEAN_LIBLA=\
	for libdir in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do 				\
		for subdir in $(LIBSSUBDIR) ; do 						\
    			[ ! -d $$libdir$$subdir ] || 						\
				find $$libdir$$subdir -name "*.la" -exec \rm -rf {} \; ; 	\
		done ;										\
	done

##
## GNU toolchains  - component information
##
export BINUTILS=binutils-2.32
export GCC=gcc-8.3.0
export GLIBC=glibc-2.30

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
	libtool 
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
	perl \
	python \
	drm    \
	gobject-introspection \
	libidn \
	gnutls \
	curl \
	git \
	xconfig \
	http/fastcgi \
	http/lighttpd \
	json \
	jq
SUBDIR+=$(EXTDIR)

## required library application folders
SUBDIR+=$(extra_SUBDIR)

##
##
## UI-related Materials - excluded from SUBDIR set. 
##
##
UIDIR_done=\
	libjpeg                \
	libpng	               \
	freetype2              \
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
	wayland-host           \
	wayland                \
	wayland-protocols      \
	xkbcommon              \
	libva                  \
	fontconfig             \
	mesa                   \
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
	pixman                 \

UIDIR=\
	vncserver              \
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
	@sudo apt --fix-broken install 
	@sudo apt update 
	@sudo apt install \
		build-essential \
		cmake automake autoconf m4 autopoint \
		unzip p7zip-full autoconf-archive \
		autogen \
		flex bison gettext net-tools texinfo \
		libssl-dev libncurses-dev \
		libmpc-dev \
		uuid-dev zlib1g-dev liblzo2-dev \
		libtool-bin pkg-config chrpath diffstat \
		gcc-multilib g++-multilib \
		gawk \
		python3 \
		python3-pip \
		python3-setuptools \
		python3-wheel \
		ninja-build \
		meson \
		qemu-system-x86 \
		libxml2-dev libffi-dev \
		help2man valgrind \
		gperf   \
		libglib2.0-dev-bin \
		ragel gengetopt
	@sudo apt --no-install-recommends install \
		xsltproc \
		xmlto \
		fop 
	@sudo pip install mako Jinja2

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
		mkdir -p $(STAGEDIR)/work \
		mkdir -p $(EXTINSTDIR); \
		mkdir -p $(EXTINSTDIR)/root; \
		mkdir -p $(EXTINSTDIR)/home; \
	)
	@[ -d $(UIXINSTDIR) ] || ( \
		mkdir -p $(UIXINSTDIR); \
		mkdir -p $(UIXINSTDIR)/root; \
		mkdir -p $(UIXINSTDIR)/home; \
		mkdir -p $(UIXINSTDIR)/include; \
		mkdir -p $(UIXINSTDIR)/lib; \
		mkdir -p $(UIXINSTDIR)/usr/local/include; \
		mkdir -p $(UIXINSTDIR)/usr/local/lib; \
		mkdir -p $(UIXINSTDIR)/usr/local/share; \
		mkdir -p $(UIXINSTDIR)/usr/local/share/aclocal; \
	)
	@[ ! -f $(BDDIR)/$(LIBPATHFILE) ] || \rm -rf $(BDDIR)/$(LIBPATHFILE)  
	@touch $(BDDIR)/$(LIBPATHFILE) 
	@for libdir in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do	\
		for subdir in $(LIBSSUBDIR) ; do 			\
			echo $$libdir$$subdir >> $(BDDIR)/$(LIBPATHFILE) ; \
			mkdir -p $$libdir$$subdir ;	 		\
		done ;							\
	done

##
## libs
##
lib: checkfirst
	@cd libs; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] || ( \
				[ -f $$dir/$(BUILDOUT) ] || touch $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) all      2>&1  | tee -a $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) install  2>&1  | tee -a $$dir/$(BUILDOUT) && \
				$(CLEAN_LIBLA) ) \
		done

lib_%: checkfirst
	@cd libs; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] ||  (     \
				[ -f $$dir/$(BUILDOUT) ] || touch $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) $(subst lib_,,$@) 2>&1 | tee $$dir/$(BUILDOUT) && \
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
				[ -f $$dir/$(BUILDOUT) ] || touch $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) all      2>&1  | tee -a $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) install  2>&1  | tee -a $$dir/$(BUILDOUT) && \
				$(CLEAN_LIBLA) ) \
		done

app_%: checkfirst
	@cd apps; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] ||  (     \
				[ -f $$dir/$(BUILDOUT) ] || touch $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(INSTALLDIR) $(subst app_,,$@) 2>&1 | tee $$dir/$(BUILDOUT) && \
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
				[ -f $$dir/$(BUILDOUT) ] || touch $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(EXTINSTDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(EXTINSTDIR) all      2>&1  | tee -a $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(EXTINSTDIR) install  2>&1  | tee -a $$dir/$(BUILDOUT) && \
				$(CLEAN_LIBLA) ) \
		done

ext_%: checkfirst
	@cd exts; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] ||  (     \
				[ -f $$dir/$(BUILDOUT) ] || touch $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(EXTINSTDIR) $(subst ext_,,$@) 2>&1 | tee $$dir/$(BUILDOUT) && \
				[ "$(subst ext_,,$@)" != "install" ] || $(CLEAN_LIBLA)  \
			) ;	\
		done

##
##  ui
##
ui: checkfirst llvm_okay
	@cd uix; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] || ( \
				[ -f $$dir/$(BUILDOUT) ] || touch $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(UIXINSTDIR) prepare  2>&1  | tee $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(UIXINSTDIR) all      2>&1  | tee -a $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(UIXINSTDIR) install  2>&1  | tee -a $$dir/$(BUILDOUT) && \
				$(CLEAN_LIBLA) ) \
		done
	
ui_%: checkfirst llvm_okay
	@cd uix; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] ||  (     \
				[ -f $$dir/$(BUILDOUT) ] || touch $$dir/$(BUILDOUT) && \
				make -C $$dir destination=$(UIXINSTDIR) $(subst ui_,,$@) 2>&1 | tee $$dir/$(BUILDOUT) && \
				[ "$(subst ui_,,$@)" != "install" ] || $(CLEAN_LIBLA)  \
			) ;	\
		done
	
##
## projects
##
proj:
	@make -C $(BDDIR)/projects destination=$(EXTINSTDIR) proj
	@$(CLEAN_LIBLA)

proj_%:
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
			[ ! -d $$dir/$(BUILDDIR) ] || \rm -rf $$dir/$(BUILDDIR); \
		done ;                                 \
		cd .. ;                                \
	done

##
##
## savediff	: source/xxx -> /patch 
## applydiff	: /patch -> source/xxx 
## wipeout	: Cleaning source/xxx
##
##
savediff:
	@for cdir in $(COMPDIR); do                 						\
		for dir in $(SUBDIR); do               						\
			if [ -f $$cdir/$$dir/Makefile ]; then 					\
				if [ -d $$cdir/$$dir/$(MICBSRC) ] &&					\
						[ ! -f $$cdir/$$dir/$(MICBSRC)/.nocleanup ]; then	\
					XDIR=`cat $$cdir/$$dir/Makefile | grep 'DIR=' | head -n 1`; 	\
					XDIR=$${XDIR#DIR=} ; 						\
					if [ -d $$cdir/$$dir/$(MICBSRC)/$$XDIR ]; then 			\
						cd $$cdir/$$dir ; 					\
						[ -d patch ] || mkdir -p patch ; 			\
						cd $(MICBSRC)/$$XDIR ; 					\
						git add -N . ; 						\
						git diff > ../../patch/$(DEV_PATCH_FILE) ;		\
						git diff --cached >> ../../patch/$(DEV_PATCH_FILE) ;	\
						echo "Diffing from $$cdir/$$dir/$(MICBSRC)/$$XDIR" ;	\
						cd ../../../../ ;				\
					fi; 							\
				fi;									\
			fi ;									\
		done ;                                						\
	done

applydiff:
	@for cdir in $(COMPDIR); do                 						\
		for dir in $(SUBDIR); do               						\
			if [ -f $$cdir/$$dir/Makefile ] && 					\
					[ -d $$cdir/$$dir/$(MICBSRC) ]; then 			\
				if [ -f $$cdir/$$dir/patch/patch.develop ]; then 		\
					XDIR=`cat $$cdir/$$dir/Makefile | grep 'DIR=' | head -n 1`; \
					XDIR=$${XDIR#DIR=} ; 					\
					if [ -d $$cdir/$$dir/$(MICBSRC)/$$XDIR ]; then 	        \
						echo "Applying to $$cdir/$$dir/$(MICBSRC)/$$XDIR" ; \
						cd $$cdir/$$dir/$(MICBSRC)/$$XDIR ; 		\
						git apply ../../patch/$(DEV_PATCH_FILE) ; 	\
						git add -N . ; 					\
						cd ../../../../ ;				\
					fi; 							\
				fi; 								\
			fi ;									\
		done ;                                						\
	done

wipeout:
	@for cdir in $(COMPDIR); do                 						\
		for dir in $(SUBDIR); do               						\
			if [ -f $$cdir/$$dir/Makefile ] &&  					\
					[ -d $$cdir/$$dir/$(MICBSRC) ]; then 			\
				XDIR=`cat $$cdir/$$dir/Makefile | grep 'DIR=' | head -n 1`; 	\
				XDIR=$${XDIR#DIR=} ; 						\
				if [ ! -f $$cdir/$$dir/$(MICBSRC)/.nocleanup ]; then		\
					\rm -rf $$cdir/$$dir/$(MICBSRC)/* ;			\
					echo "Cleaning in $$cdir/$$dir/$(MICBSRC)/*" ;		\
				fi;								\
			fi ;									\
		done ;                                						\
	done

##
## extdisk
## extdisk_clean
##
extdisk:
	@cd gnu; make -f Makefile hdrpath=$(EXTINSTDIR) setup_headers
	@$(TOPDIR)/scripts/setupdisk.sh build $(STAGEDIR) $(EXTDISK) $(BDDIR)/$(EXTDISKNM) $(EXTDISKBLKS)

extdisk_clean:
	@$(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(EXTDISKNM)
			

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
	@make -C apps/busybox destination=$(XBASEDIR) -f Makefile.bootstrap prepare all install
	@$(shell $(BUILDUP_ROOTFS)) > /dev/null
	@chmod ugo+x $(XBASEDIR)/etc/init.d/rcS
	@chmod ugo+x $(XBASEDIR)/etc/init.d/rc.shutdown

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
	@if [ "$(subst kernel_,,$@)" = "all" ] ; then make -C modules KERNDIR=$(KERNDIR) all install; fi
	@if [ "$(subst kernel_,,$@)" = "clean" ] ; then \
		echo ""	                     ; \
		echo "Cleaning image..."     ; \
		echo ""                      ; \
	       	make -C $(BDDIR) clean       ; \
	fi

mod_%:
	@make KERNDIR=$(KERNDIR) -C modules $(subst mod_,,$@)
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
	@echo "Building private modules..."
	@echo ""
	@make -C modules KERNDIR=$(KERNDIR) all install
	@echo ""
	@echo "Merging $(INSTALLDIR) and GLIBC ...."
	@echo ""
	@[ ! -d $(FINDIR) ] || sudo \rm -rf $(FINDIR)
	@mkdir -p $(FINDIR)
	@cd $(XBASEDIR) ; \
		sudo find . -print -depth | sudo cpio -pdm $(FINDIR) ; \
		cd $(TOPDIR) ; \
		make -C apps/glibc destination=$(FINDIR)/$(INSTSUBFN) install_glibc
	@cd $(FINDIR)/$(INSTSUBFN)/lib ; \
		for fn in $(BOOTSTRAP_LIBS); do [ -s $$fn ] || ln -s ../lib/$$fn $$fn ; \
		done
	@echo ""
	@echo "Library copying --> for bootstrap use "
	@echo ""
	@( \
		cd $(FINDIR)/$(INSTSUBFN)/lib; \
			tar jcvf $(TEMPFN).tar.bz2 $(BOOTSTRAP_LIBS) > /dev/null ; \
			tar jxvf $(TEMPFN).tar.bz2 -C $(FINDIR)/lib  > /dev/null ; \
			\rm -rf  $(TEMPFN).tar.bz2 ;  \
		echo "Done"  \
	)

ramdisk:
	@echo ""
	@echo "Compiling kernel and combining squash image... "
	@echo ""
	@make -C $(BDDIR)/kernel destination=$(FINDIR) isodir=$(ISODIR) all install
	@echo ""
	@echo "Build up image..."
	@echo ""
	@make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) install

reformat:
	@[ ! -d $(XBASEDIR)   ] || \rm -rf $(XBASEDIR)
	@[ ! -d $(STAGEDIR)   ] || \rm -rf $(STAGEDIR)
	@[ ! -d $(FINDIR)     ] || \rm -rf $(FINDIR)
	@[ ! -d $(ISODIR)     ] || \rm -rf $(ISODIR)
	@make -C $(BDDIR)        uninstall
	@make -C $(BDDIR)/kernel uninstall
	@cd $(BDDIR); \
		$(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(EXTDISKNM)

toolchain:
	@echo ""
	@echo ""
	@echo "Preparing sources.... "
	@echo ""
	@echo ""
	@make -C gnu/sources -f ../Makefile download
	@echo ""
	@echo ""
	@echo "BINUTILS....."
	@echo ""
	@echo ""
	@[ -d gnu/binutils/build/$(_ARCH_) ] || mkdir -p gnu/binutils/build/$(_ARCH_)
	@cd gnu/binutils/build/$(_ARCH_); \
		make -f ../../../Makefile config_binutils setup_binutils
	@echo ""
	@echo ""
	@echo "Kernel Headers....."
	@echo ""
	@echo ""
	@[ -d $(KERNDIR)/$(KERNELVER) ] || \
		make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) prepare
	@cd gnu; \
		make -f Makefile hdrpath=$(TOOLCHAIN_ROOT)/$(PLATFORM) setup_headers 
	@[ ! -f $(TOOLCHAIN_ROOT)/$(PLATFORM)/include/asm/.install ] || ( \
		echo ""  ;                     \
		echo "Cleaning up kernel folder" ; \
		echo ""  ;                     \
		\rm -rf $(KERNDIR)/$(KERNELVER) ; )
	@echo ""
	@echo "GCC Bootstrap ....."
	@echo ""
	@echo ""
	@[ -d gnu/gcc/build/$(_ARCH_) ] || mkdir -p gnu/gcc/build/$(_ARCH_)
	@cd gnu/gcc/build/$(_ARCH_); \
		make -f ../../../Makefile config_gcc setup_gcc
	@echo ""
	@echo ""
	@echo "glibc Bootstrap ....."
	@echo ""
	@echo ""
	@[ -d gnu/glibc/build/$(_ARCH_) ] || mkdir -p gnu/glibc/build/$(_ARCH_)
	@cd gnu/glibc/build/$(_ARCH_); \
		make -f ../../../Makefile config_glibc setup_glibc
	@echo ""
	@echo ""
	@echo "GCC Host Compiler....."
	@echo ""
	@echo ""
	@cd gnu/gcc/build/$(_ARCH_); \
		make -f ../../../Makefile setup_gcc_2
	@echo ""
	@echo ""
	@echo "glibc Host Library....."
	@echo ""
	@echo ""
	@cd gnu/glibc/build/$(_ARCH_); \
		make -f ../../../Makefile setup_glibc_2
	@echo ""
	@echo ""
	@echo "GCC Extra Libraries....."
	@echo ""
	@echo ""
	@cd gnu/gcc/build/$(_ARCH_); \
		make -f ../../../Makefile setup_gcc_3

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

	
