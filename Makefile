## current directory
export TOPDIR=$(shell pwd)

## by default ubuntu platform 
export TBOARD=unknown

# boards selection
export BDDIR=$(TOPDIR)/boards/$(TBOARD)

# Host system name such as "x86_64-linux" or "i686-linux"
export HOSTSYSTEM=$(shell echo `uname -m`-`uname -s` | awk '{print tolower($$0)}')

# Verbose mode compilation
export QUEIT=@

#
# Current Linux Ubuntu version
#
# This has been tested with either Ubuntu 22.04 or Ubuntu 24.04
#
export UBUNTU_VERSION=$(firstword $(subst ., ,$(shell echo `lsb_release -a | grep Description | awk '{print $$3}'`)))

##
## Environment variable LD_LIBRARY_PATH mnakes an error impact on toolchain build.
##
export ENV_CHECK_CMD=$(shell echo `export | grep LD_LIBRARY_PATH | wc -l`)

#
# Python version in current system
#
export PYTHON_VER1=$(word 1, $(subst ., ,$(shell echo `python3 --version | awk '{print $$2}'`)))
export PYTHON_VER2=$(word 2, $(subst ., ,$(shell echo `python3 --version | awk '{print $$2}'`)))
export PYTHON_VER3=$(word 3, $(subst ., ,$(shell echo `python3 --version | awk '{print $$2}'`)))
export PYTHON_SYSVER=$(PYTHON_VER1).$(PYTHON_VER2)
export PYTHON_SYSREV=$(PYTHON_VER3)

# Patch file name 
export DEV_PATCH_FILE=patch.develop

# direcoty list
include $(TOPDIR)/dir.inc

# board specific environment 
include $(BDDIR)/env.mk

#
# "vm" mode needs external USB disk/image file keeping "image.ext4" of /board/vm/.
# USB device node/file image  should be passed to boards/vm/Makefile . 
#
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
## "board" output file 
export BOARD_BUILDOUT=$(TOPDIR)/boards/$(TBOARD)/build_log.$(_ARCH_)

##
## Download history file
export DNOUT=$(BUILDDIR)/download.log

##
## Build output file 
export BUILDOUT=$(BUILDDIR)/build.log

define SETUP_BUILDOUT
	[ -d $(1)/$(BUILDDIR) ] || mkdir -p $(1)/$(BUILDDIR)  && \
	[ -f $(1)/$(BUILDOUT) ] || touch $(1)/$(BUILDOUT)
endef

##
## Cleanup for "distclean" - parsing 'DIR=' string in Makefile.
##
define CLEANUP_BUILDDIR
	$(eval TDIR := $$(1))                                                             \
	$(eval XDIR := `cat $$(1)/Makefile | grep "DIR=" | head -n 1 | sed "s/DIR=//"`)   \
	[ ! -d $(TDIR)/$(BUILDDIR)/$(XDIR) ] || (                                         \
		\rm -rf $(TDIR)/$(BUILDDIR)/$(XDIR)/*    &&                                   \
		\rm -rf $(TDIR)/$(BUILDDIR)/$(XDIR)/.*   )
endef

##
## Build lock folder
##
export BUILD_FOLDER=/var/tmp/microbsp
export BUILD_LOCK_FOLDER=$(BUILD_FOLDER)/lock
export GLOBAL_LOCK_FILE=/var/tmp/microbsp-global-lock-file

##
## exclusive execution of command
##
define PERFORM_EXCLUSIVELY
	$(eval DEPTH  := `echo $$2 | sed "s/\//-/g"`)         \
	$(eval LOCKFN := $(BUILD_LOCK_FOLDER)/microbsp-$1-)   \
	[ -f $(LOCKFN)$(DEPTH).$(3) ] || touch $(LOCKFN)$(DEPTH).$(3)  && \
	flock $(LOCKFN)$(DEPTH).$(3) $(4)
endef

## Exclusive execution
define DO_EXCL
	$(call PERFORM_EXCLUSIVELY,$3,$1,$2,\
			make -C $1 destination=$4 $2 2>&1  | tee -a $1/$(BUILDOUT) )
endef

## Exclusive execution for downloading ..
define DO_EXCL_DN
	$(call PERFORM_EXCLUSIVELY,$3,$1,$2,\
			make -C $1 destination=$4 $2 2>&1  | tee -a $1/$(DNOUT) )
endef

## Exclusive execution - Only single execution in system
define DO_EXCL_SINGLE
	[ -f $(GLOBAL_LOCK_FILE) ] || touch $(GLOBAL_LOCK_FILE) && \
	flock $(GLOBAL_LOCK_FILE) $(1)
endef

## Normal execution
define DO_NORM
	make -C $1 destination=$4 $2 2>&1 | tee -a $1/$(BUILDOUT)
endef

## ISO image name...
export IMAGENAME=output.iso

## install dir
INSTSUBFN=disk
XBASEDIR=$(BDDIR)/_basedir
FINDIR=$(BDDIR)/_tempdir
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

## Library directories
ifneq (,$(findstring libs,$(COMPDIR)))
SUBDIR+=$(_LIBDIR_)
endif

## Application directories
ifneq (,$(findstring apps,$(COMPDIR)))
SUBDIR+=$(_APPDIR_)
endif

# External application directories
ifneq (,$(findstring exts,$(COMPDIR)))
SUBDIR+=$(_EXTDIR_)
endif

# UI directories
ifneq (,$(findstring uix,$(COMPDIR)))
SUBDIR+=$(_UIDIR_)
endif

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
	$(QUEIT)echo ""
	$(QUEIT)echo ""
	$(QUEIT)echo "Installing required SW sets..."
	$(QUEIT)echo "Tested/verfied in Ubuntu 20.04/22.04/24.04."
	$(QUEIT)echo ""
	$(QUEIT)echo ""
	$(QUEIT)sudo apt --fix-broken install 
	$(QUEIT)sudo apt update 
	$(QUEIT)sudo apt install -y --no-install-recommends \
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
		gtk-doc-tools       uml-utilities         bridge-utils    genisoimage       \
		python3-xcbgen      python2               xorriso         mtools            \
		device-tree-compiler swig                 triehash                          \
		xsltproc 			xmlto 			      fop
	$(QUEIT)pip install --break-system-packages --user                              \
		pkgconfig mako Jinja2 package_name setuptools

##
## Environment check
##
env_check:
	$(QUEIT)[ "$(ENV_CHECK_CMD)" = "0" ] || ( \
		echo "";                                                        \
		echo "LD_LIBRARY_PATH shouldn't be in the environment setup." ; \
		echo "";                                                        \
		echo "# unset LD_LIBRARY_PATH ";                                \
		echo "";                                                        \
		exit 1                                                          \
	)

##
## Currently only ubuntu 22.04 and 24.04 are supported !!
##
ubuntu_check:
	$(QUEIT)[ "$(UBUNTU_VERSION)" = "24" -o "$(UBUNTU_VERSION)" = "22" ] || ( \
			echo "";                                                                 \
			echo "This was verified with only Ubuntu distribution 22.04 and 24.04."; \
			echo "";                                                                 \
			exit 1                                                                   \
	)

compiler_check:
	$(QUEIT)[ "$(CHECK_TOOLCHAIN)" = "0" ] || ( \
		[ -f $(TOOLCHAIN_ROOT)/bin/$(CC) ] || ( \
			echo "";                                                                                 \
			echo "Two more cross compilers $(CC) have been installed in the system. Please check. "; \
			echo "";                                                                                 \
			exit 1                                                                                   \
		) )
	$(QUEIT)[ "$(CHECK_TOOLCHAIN)" != "0" ] || ( \
		echo "";                                                                       \
		echo "$(CC) seems not to be ready" ;                                           \
		if [ ! -f $(TOOLCHAIN_ROOT)/bin/$(CC) ]; then                                  \
			echo "Please run \"make toolchain\" first. It will take long time. " ;     \
		else                                                                           \
			echo "Please run \"export PATH=\$$PATH:$(TOOLCHAIN_ROOT)/bin\" simply." ;  \
		fi;                                                                            \
		echo "" ;                                                                      \
		exit 1 )

checkfirst: compiler_check ubuntu_check env_check
	$(QUEIT)if [ ! -d $(INSTALLDIR) ]; then \
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
	$(QUEIT)[ -d $(STAGEDIR) ] || ( \
		mkdir -p $(STAGEDIR); \
		mkdir -p $(STAGEDIR)/up;   \
		mkdir -p $(STAGEDIR)/work; \
		mkdir -p $(STAGEDIR)/usr/root; \
		mkdir -p $(STAGEDIR)/usr/home; \
		mkdir -p $(EXTINSTDIR); \
	)
	$(QUEIT)[ -d $(UIXINSTDIR) ] || ( \
		mkdir -p $(UIXINSTDIR); \
		mkdir -p $(UIXINSTDIR)/include; \
		mkdir -p $(UIXINSTDIR)/lib; \
		mkdir -p $(UIXINSTDIR)/share ; \
		mkdir -p $(UIXINSTDIR)/local/include; \
		mkdir -p $(UIXINSTDIR)/local/lib; \
		mkdir -p $(UIXINSTDIR)/local/share; \
		mkdir -p $(UIXINSTDIR)/local/share/aclocal; \
	)
	$(QUEIT)[ ! -f $(BDDIR)/$(LIBPATHFILE) ] || \rm -rf $(BDDIR)/$(LIBPATHFILE)
	$(QUEIT)touch $(BDDIR)/$(LIBPATHFILE)
	$(QUEIT)for libdir in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do	\
		for subdir in $(LIBSSUBDIR) ; do 			\
			echo $$libdir$$subdir >> $(BDDIR)/$(LIBPATHFILE) ; \
			mkdir -p $$libdir$$subdir ;	 		\
		done ;							\
	done
	$(QUEIT)[ -d $(BUILD_FOLDER)      ] || mkdir -p $(BUILD_FOLDER)
	$(QUEIT)[ -d $(BUILD_LOCK_FOLDER) ] || mkdir -p $(BUILD_LOCK_FOLDER)

##
## libs
##
lib: checkfirst
	$(QUEIT)cd libs; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] || (          \
				$(call SETUP_BUILDOUT,$$dir)                     && \
				$(call DO_EXCL,$$dir,prepare,lib,$(INSTALLDIR))  && \
				$(call DO_NORM,$$dir,all,lib,$(INSTALLDIR))      && \
				$(call DO_NORM,$$dir,install,lib,$(INSTALLDIR))  && \
				$(CLEAN_LIBLA) ) \
		done

##
## lib_[download/prepare/all/install]
##
lib_%: checkfirst
	$(QUEIT)cd libs; \
		for dir in $(SUBDIR); do                     \
			[ ! -d $$dir ] ||  (                     \
				$(call SETUP_BUILDOUT,$$dir)      && \
				if [ "$(subst lib_,,$@)" = "prepare" ]; then \
					$(call DO_EXCL,$$dir,$(subst lib_,,$@),lib,$(INSTALLDIR)) ;  \
				elif [ "$(subst lib_,,$@)" = "download" ]; then \
					$(call DO_EXCL_DN,$$dir,$(subst lib_,,$@),lib,$(INSTALLDIR)) ;  \
				else  \
					$(call DO_NORM,$$dir,$(subst lib_,,$@),lib,$(INSTALLDIR)) ;  \
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
			[ ! -d $$dir ] || (          \
				$(call SETUP_BUILDOUT,$$dir)                     && \
				$(call DO_EXCL,$$dir,prepare,app,$(INSTALLDIR))  && \
				$(call DO_NORM,$$dir,all,app,$(INSTALLDIR))      && \
				$(call DO_NORM,$$dir,install,app,$(INSTALLDIR))  && \
				$(CLEAN_LIBLA) ) \
		done

##
## app_[download/prepare/all/install]
##
app_%: checkfirst
	@cd apps; \
		for dir in $(SUBDIR); do                     \
			[ ! -d $$dir ] ||  (                     \
				$(call SETUP_BUILDOUT,$$dir)      && \
				if [ "$(subst app_,,$@)" = "prepare" ]; then                        \
					$(call DO_EXCL,$$dir,$(subst app_,,$@),app,$(INSTALLDIR)) ;     \
				elif [ "$(subst app_,,$@)" = "download" ]; then                     \
					$(call DO_EXCL_DN,$$dir,$(subst app_,,$@),app,$(INSTALLDIR)) ;  \
				else                                                                \
					$(call DO_NORM,$$dir,$(subst app_,,$@),app,$(INSTALLDIR)) ;     \
				fi && \
				[ "$(subst app_,,$@)" != "install" ] || $(CLEAN_LIBLA)  \
			) ;	\
		done

##
## exts
##
ext: checkfirst
	$(QUEIT)cd exts; \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] || (          \
				$(call SETUP_BUILDOUT,$$dir)                     && \
				$(call DO_EXCL,$$dir,prepare,ext,$(EXTINSTDIR))  && \
				$(call DO_NORM,$$dir,all,ext,$(EXTINSTDIR))      && \
				$(call DO_NORM,$$dir,install,ext,$(EXTINSTDIR))  && \
				$(CLEAN_LIBLA) ) \
		done

##
## ext_[download/prepare/all/install]
##
ext_%: checkfirst
	$(QUEIT)cd exts; \
		for dir in $(SUBDIR); do                     \
			[ ! -d $$dir ] ||  (                     \
				$(call SETUP_BUILDOUT,$$dir)      && \
				if [ "$(subst ext_,,$@)" = "prepare" ]; then                        \
					$(call DO_EXCL,$$dir,$(subst ext_,,$@),ext,$(EXTINSTDIR)) ;     \
				elif [ "$(subst ext_,,$@)" = "download" ]; then                     \
					$(call DO_EXCL_DN,$$dir,$(subst ext_,,$@),ext,$(EXTINSTDIR)) ;  \
				else                                                                \
					$(call DO_NORM,$$dir,$(subst ext_,,$@),ext,$(EXTINSTDIR)) ;     \
				fi && \
				[ "$(subst ext_,,$@)" != "install" ] || $(CLEAN_LIBLA)  \
			) ;	\
		done

##
## uix
##
ui: checkfirst llvm_okay
	$(QUEIT)cd uix && \
		for dir in $(SUBDIR); do         \
			[ ! -d $$dir ] || (          \
				$(call SETUP_BUILDOUT,$$dir)                     && \
				$(call DO_EXCL,$$dir,prepare,ui,$(UNIXINSTDIR))  && \
				$(call DO_NORM,$$dir,all,ui,$(UIXINSTDIR))       && \
				$(call DO_NORM,$$dir,install,ui,$(UIXINSTDIR))   && \
				$(CLEAN_LIBLA) ) \
		done


##
## ui_[prepare/all/install]
##
ui_%: checkfirst llvm_okay
	$(QUEIT)cd uix; \
		for dir in $(SUBDIR); do                     \
			[ ! -d $$dir ] ||  (                     \
				$(call SETUP_BUILDOUT,$$dir)      && \
				if [ "$(subst ui_,,$@)" = "prepare" ]; then                        \
					$(call DO_EXCL,$$dir,$(subst ui_,,$@),ui,$(UIXINSTDIR)) ;     \
				elif [ "$(subst ui_,,$@)" = "download" ]; then                     \
					$(call DO_EXCL_DN,$$dir,$(subst ui_,,$@),ui,$(UIXINSTDIR)) ;  \
				else                                                               \
					$(call DO_NORM,$$dir,$(subst ui_,,$@),ui,$(UIXINSTDIR)) ;      \
				fi &&  \
				[ "$(subst ui_,,$@)" != "install" ] || $(CLEAN_LIBLA)  \
			) ;	\
		done

##
## projects
##
proj: checkfirst 
	$(QUEIT)make -C $(BDDIR)/projects destination=$(EXTINSTDIR) prepare all install
	$(QUEIT)$(CLEAN_LIBLA)

proj_%: checkfirst 
	$(QUEIT)make -C $(BDDIR)/projects destination=$(EXTINSTDIR) $(subst proj_,,$@)
	$(QUEIT)$(CLEAN_LIBLA)


##
## LLVM compiler -- reuqired by UI sets...
##
llvm_okay:
	$(QUEIT)[ -f $(TOOLCHAIN_ROOT)/bin/$(PLATFORM)-gcc ] || ( \
		echo ""; \
		echo "Base cross-compiler should be installed " ; \
		echo ""; \
		exit 1 \
		)	

llvm: llvm_okay
	$(QUEIT)make -C gnu/llvm prepare all install
	$(QUEIT)$(CLEAN_LIBLA)

llvm_%: llvm_okay
	$(QUEIT)make -C gnu/llvm $(subst llvm_,,$@)
	$(QUEIT)$(CLEAN_LIBLA)


##
## clean
##
clean: checkfirst
	$(QUEIT)for cat in $(COMPDIR); do                 \
		cd $$cat ;                             \
		for dir in $(SUBDIR); do               \
			[ ! -d $$dir ] || make -C $$dir clean || exit 1; \
			[ ! -f $$dir/$(BUILDOUT) ] || \rm -f $$dir/$(BUILDOUT) ; \
		done ;                                 \
		cd .. ;                                \
	done

##
## distclean
##
distclean: checkfirst
	$(QUEIT)for cat in $(COMPDIR); do                 \
		cd $$cat ;                             \
		for dir in $(SUBDIR); do               \
			[ ! -d $$dir ] || (                \
				$(call CLEANUP_BUILDDIR,$$dir)                                   ; \
				[ ! -f $$dir/$(BUILDOUT) ] || \rm -f $$dir/$(BUILDOUT)           ; \
				[ ! -f $$dir/$(LIBFLAGS_NAME) ] || rm -f $$dir/$(LIBFLAGS_NAME)  ; \
				[ ! -f $$dir/$(INCFLAGS_NAME) ] || rm -f $$dir/$(INCFLAGS_NAME)  ; \
			)                                  \
		done ;                                 \
		cd .. ;                                \
	done

##
## download
##
download_prologue:
	$(QUEIT)make -C gnu/sources -f ../Makefile download
	$(QUEIT)[ -d $(KERNDIR)/$(KERNELVER) ] || make -C $(BDDIR)/kernel prepare
	$(QUEIT)for cat in $(COMPDIR); do                 \
		cd $$cat ;                             \
		for dir in $(SUBDIR); do               \
			[ ! -d $$dir ] || (                \
				[ -f $$dir/$(DNOUT) ] || touch $$dir/$(DNOUT)  ; \
			)                                  \
		done ;                                 \
		cd .. ;                                \
	done

download_epilogue:
	$(QUEIT)for cat in $(COMPDIR); do                 \
		cd $$cat ;                             \
		for dir in $(SUBDIR); do               \
			[ ! -d $$dir ] || (                \
				[ ! -f $$dir/$(BUILDOUT) ] || \rm -f $$dir/$(BUILDOUT)         ; \
				[ ! -f $$dir/$(LIBFLAGS_NAME) ] || rm -f $$dir/$(LIBFLAGS_NAME); \
				[ ! -f $$dir/$(INCFLAGS_NAME) ] || rm -f $$dir/$(INCFLAGS_NAME); \
			)                                  \
		done ;                                 \
		cd .. ;                                \
	done

download: compiler_check download_prologue lib_download app_download ext_download ui_download download_epilogue

##
## cleanup everything 
##
wipeout:
	$(QUEIT)for cdir in $(COMPDIR); do                 						        \
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
			[ ! -f $$cdir/$$dir/$(DNOUT) ] || \rm -rf $$cdir/$$dir/$(DNOUT) ;   \
		done ;                                						            \
	done

##
## extdisk
## extdisk_clean
##
extdisk: compiler_check
	$(QUEIT)cd gnu; make -f Makefile hdrpath=$(EXTINSTDIR) setup_headers
	$(QUEIT)$(call DO_EXCL_SINGLE,\
	       $(TOPDIR)/scripts/setupdisk.sh build $(STAGEDIR) $(EXTDISK) $(BDDIR)/$(EXTDISKNM) $(EXTDISKBLKS) )
	$(QUEIT)[ -z $(CFGDISKNM) ] || sudo dd if=/dev/zero of=$(EXT4CFG) bs=1M count=$(CFGDISKBLKS)
	$(QUEIT)[ -z $(CFGDISKNM) ] || sudo mkfs.ext4 -j -F -L $(CFGVOLNM) $(EXT4CFG)


extdisk_clean: compiler_check 
	$(QUEIT)$(call DO_EXCL_SINGLE, \
		$(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(EXTDISKNM))
	$(QUEIT)[ -z $(CFGDISKNM) ] || ( [ ! -f $(BDDIR)/$(CFGDISKNM) ] || \rm -rf $(BDDIR)/$(CFGDISKNM) )
			

##
## uidisk
## uidisk_clean
##
uidisk: compiler_check 
	$(QUEIT)[ "$(UIDISK)" = "" ] ||  ( \
		$(call DO_EXCL_SINGLE,\
			$(TOPDIR)/scripts/setupdisk.sh build $(BDDIR)/_uidir $(UIDISK) $(BDDIR)/$(UIDISKNM) $(UIDISKBLKS)) )

uidisk_clean: compiler_check 
	$(QUEIT)[ "$(UIDISK)" = "" ] ||  ( \
		$(call DO_EXCL_SINGLE,\
			$(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(UIDISKNM) ) )
			

##
## BUILDUP_ROOTFS --> Please refer to boards/$(TBOARD)/env.mk 
##
run_bootstrap: checkfirst
	$(QUEIT)[ -f $(BOARD_BUILDOUT) ] || touch $(BOARD_BUILDOUT)
	$(QUEIT)make -C apps/busybox  destination=$(XBASEDIR) -f Makefile.bootstrap download prepare all install   | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)make -C apps/bash     destination=$(XBASEDIR) install                                              | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)$(shell $(BUILDUP_ROOTFS)) > /dev/null                                                             | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)[ ! -f $(XBASEDIR)/etc/init.d/rcS ]         || chmod ugo+x $(XBASEDIR)/etc/init.d/rcS
	$(QUEIT)[ ! -f $(XBASEDIR)/etc/init.d/rc.shutdown ] || chmod ugo+x $(XBASEDIR)/etc/init.d/rc.shutdown

boot_%: checkfirst
	$(QUEIT)echo ""
	$(QUEIT)echo "Bootloader configuration/cleanup"
	$(QUEIT)echo ""
	$(QUEIT)make -C $(BDDIR)/boot isodir=$(ISODIR) $(subst boot_,,$@)

kernel_%: checkfirst
	$(QUEIT)echo ""
	$(QUEIT)echo "Kernel configuration/cleanup"
	$(QUEIT)echo ""
	$(QUEIT)make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) $(subst kernel_,,$@)
	$(QUEIT)if [ "$(subst kernel_,,$@)" = "clean" ] ; then \
		echo ""	                     ; \
		echo "Cleaning image..."     ; \
		echo ""                      ; \
	       	make -C $(BDDIR) clean       ; \
	fi

modules_%: checkfirst
	$(QUEIT)if [ "$(subst mod_,,$@)" = "install" ] ; then \
		make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) install ;              \
		make -C $(BDDIR)        destination=$(INSTALLDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) install ; \
	fi

board: run_bootstrap
	$(QUEIT)[ ! -f $(BOARD_BUILDOUT) ] || rm $(BOARD_BUILDOUT)
	$(QUEIT)touch $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Preparing board ..."                                               | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)make -C $(BDDIR) destination=$(INSTALLDIR) isodir=$(ISODIR) prepare                    2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Building kernel ..."                                               | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) dnfw prepare all    2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "External modules ..."                                              | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)make -C $(BDDIR)/kernel KERNDIR=$(KERNDIR) destination=$(INSTALLDIR) extmod            2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Copying static rootfs ..."                                         | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)cp -rf $(BDDIR)/rootfs/* $(INSTALLDIR)/
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Collecting $(INSTALLDIR) and GLIB to $(FINDIR) ..."                | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)[ ! -d $(FINDIR) ] || \rm -rf $(FINDIR)
	$(QUEIT)mkdir -p $(FINDIR)
	$(QUEIT)cd $(XBASEDIR) ; find . -depth -print | cpio -pdm $(FINDIR)
	$(QUEIT)make -C apps/glibc destination=$(FINDIR)/$(INSTSUBFN) install_glibc      2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Setting for copying $(BOOTSTRAP_LIBS) and $(BOOTSTRAP_LDRS) ... "  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)cd $(FINDIR)/$(INSTSUBFN)/lib ; \
		for fn in $(BOOTSTRAP_LIBS); do [ ! -f $$fn ] || cp -f $$fn ../../lib/ ; done
	$(QUEIT)cd $(FINDIR); \
		[ -d $(dir $(BOOTSRTAP_LDRS)) ] || mkdir -p $(dir $(BOOTSTRAP_LDRS))
	$(QUEIT)cd $(FINDIR)/$(INSTSUBFN); \
		[ -d $(dir $(BOOTSRTAP_LDRS)) ] || mkdir -p $(dir $(BOOTSTRAP_LDRS))
	$(QUEIT)echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Copying $(notdir $(BOOTSTRAP_LDRS)) to $(FINDIR)/$(INSTSUBFN)/lib64 ..."         | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)[ -f $(FINDIR)/$(INSTSUBFN)/lib64/$(notdir $(BOOTSTRAP_LDRS)) ] || \
		cp -rf  $(INSTALLED_LDRS)  $(FINDIR)/$(INSTSUBFN)/lib64
	$(QUEIT)echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Copying $(notdir $(BOOTSTRAP_LDRS)) to $(FINDIR)/lib64 ..."                      | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)[ -f $(FINDIR)/lib64/$(notdir $(BOOTSTRAP_LDRS)) ]              || \
		cp -rf  $(INSTALLED_LDRS)  $(FINDIR)/lib64
	$(QUEIT)echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Copying $(notdir $(BOOTSTRAP_LDRS)) to $(FINDIR)/lib ..."                        | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)[ -f $(FINDIR)/lib/$(notdir $(BOOTSTRAP_LDRS)) ]                || \
		cp -rf  $(INSTALLED_LDRS)  $(FINDIR)/lib

ramdisk: checkfirst
	$(QUEIT)[ -f $(BOARD_BUILDOUT) ] || touch $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                                2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Compiling kernel and combining squash image ..."                                 2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                                2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)make -C $(BDDIR)/kernel destination=$(FINDIR) isodir=$(ISODIR) install                 2>&1 | tee -a $(BOARD_BUIDLOUT)
	$(QUEIT)echo ""                                                                                2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo "Build up image ..."                                                              2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)echo ""                                                                                2>&1 | tee -a $(BOARD_BUILDOUT)
	$(QUEIT)make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) install                         2>&1 | tee -a $(BOARD_BUILDOUT)

cleanup:
	$(QUEIT)[ ! -d $(XBASEDIR)   ] || \rm -rf $(XBASEDIR)
	$(QUEIT)[ ! -d $(STAGEDIR)   ] || \rm -rf $(STAGEDIR)
	$(QUEIT)[ ! -d $(UIXINSTDIR) ] || \rm -rf $(UIXINSTDIR)
	$(QUEIT)[ ! -d $(FINDIR)     ] || \rm -rf $(FINDIR)
	$(QUEIT)[ ! -f $(BOARD_BUILDOUT)  ] || \rm -rf $(BOARD_BUILDOUT)
	$(QUEIT)make -C $(BDDIR)        isodir=$(ISODIR) isoname=$(IMAGENAME) uninstall
	$(QUEIT)make -C $(BDDIR)/kernel uninstall
	$(QUEIT)cd $(BDDIR); \
		$(call DO_EXCL_SINGLE,\
			[ ! -f $(BDDIR)/$(EXTDISKNM) ] || $(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(EXTDISKNM) )
	$(QUEIT)cd $(BDDIR); \
		$(call DO_EXCL_SINGLE,\
			[ ! -f $(BDDIR)/$(CFGDISKNM) ] || $(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(CFGDISKNM) )

toolchain: ubuntu_check env_check
	$(QUEIT)[ -f $(TOOLCHAIN_BUILDOUT) ] || touch $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo "Preparing sources ... "                2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)$(call DO_EXCL_SINGLE,make -C gnu/sources -f ../Makefile download)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo "BINUTILS ..."                          2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)[ -d gnu/binutils/build/$(_ARCH_)/$(BINUTILS) ] || mkdir -p gnu/binutils/build/$(_ARCH_)/$(BINUTILS)
	$(QUEIT)cd gnu/binutils/build/$(_ARCH_)/$(BINUTILS); \
		$(call DO_EXCL_SINGLE, \
			make -f $(TOPDIR)/gnu/Makefile config_binutils 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)) && \
		make -f $(TOPDIR)/gnu/Makefile setup_binutils  2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo "Kernel Headers ..."                    2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)[ -d $(KERNDIR)/$(KERNELVER) ] || \
		make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) prepare 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)cd gnu; \
		$(call DO_EXCL_SINGLE, \
			make -f Makefile hdrpath=$(TOOLCHAIN_ROOT)/$(PLATFORM) setup_headers 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT))
	$(QUEIT)[ ! -f $(TOOLCHAIN_ROOT)/$(PLATFORM)/include/asm/.install ] || ( \
		echo ""  ;                     \
		echo "Cleaning up kernel folder ..." ; \
		echo ""  ;                     \
		\rm -rf $(KERNDIR)/$(KERNELVER) ; )
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo "GCC Bootstrap ..."                     2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)[ -d gnu/gcc/build/$(_ARCH_)/$(GCC) ] || mkdir -p gnu/gcc/build/$(_ARCH_)/$(GCC)
	$(QUEIT)cd gnu/gcc/build/$(_ARCH_)/$(GCC); \
		$(call DO_EXCL_SINGLE, \
			make -f $(TOPDIR)/gnu/Makefile config_gcc 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)) && \
		make -f $(TOPDIR)/gnu/Makefile setup_gcc  2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo "GLIBC Bootstrap ..."                   2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)[ -d gnu/glibc/build/$(_ARCH_)/$(GLIBC) ] || mkdir -p gnu/glibc/build/$(_ARCH_)/$(GLIBC)
	$(QUEIT)cd gnu/glibc/build/$(_ARCH_)/$(GLIBC); \
		$(call DO_EXCL_SINGLE, \
			make -f $(TOPDIR)/gnu/Makefile config_glibc 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)) && \
		make -f $(TOPDIR)/gnu/Makefile setup_glibc      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo "GCC Host Compiler ..."                 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)cd gnu/gcc/build/$(_ARCH_)/$(GCC); \
		make -f $(TOPDIR)/gnu/Makefile setup_gcc_2     2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo "GLIBC Host Library ..."                2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)cd gnu/glibc/build/$(_ARCH_)/$(GLIBC); \
		make -f $(TOPDIR)/gnu/Makefile setup_glibc_2   2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo "GCC Extra Libraries ..."               2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	$(QUEIT)cd gnu/gcc/build/$(_ARCH_)/$(GCC); \
		make -f $(TOPDIR)/gnu/Makefile setup_gcc_3     2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)

#########################################################################################################################
#
#  VM execution 
#
#  May/19/2024 : Only x86_64 VM mode (TBOARD=vm) is available for this option. 
#                Plese refer to board/vm/Makefile. It depends on qemu-system-x86 package. 
#                # make TBOARD=vm [vmrun|vmstop] 
#
#########################################################################################################################

vm%: ubuntu_check
	@if [ "$(subst vm,,$@)" = "run" ]; then                                \
		make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) run ;       \
	elif [ "$(subst vm,,$$@)" = "stop" ]; then                              \
		make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) stop ;      \
	else                                                                   \
		echo "Only two commands; vmrun vmstop " ;                          \
	fi

pkglist: ubuntu_check
	$(QUEIT)for dir in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do 	\
		for subdir in $(LIBSSUBDIR) ; do 			            \
	    		[ ! -d $$dir$$subdir/pkgconfig ] || 		    \
				find $$dir$$subdir -name "*.pc" ; 	            \
			done ;						                        \
		done

folders:
	$(QUEIT)echo $(SUBDIR)


change: 
	for grp in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do \
		for dir in $(LIBSSUBDIR) ; do  \
			if [ -f $$grp$$dir/pkgconfig/*.pc ] ; then \
				echo "
				$(TOPDIR)/scripts/filterlibs.sh \
					$(BDDIR) $$grp$$dir/pkgconfig/$$dep.pc $$grp$$dir ; \
			fi ; \
		done ; \
	done
