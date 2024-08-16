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
## It means that we are now supporting Ubuntu-22.04.
##
export PYTHON_SYSVER=3.10

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
	@sudo apt install -y \
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
		device-tree-compiler swig                 triehash
	@sudo apt --no-install-recommends install \
		xsltproc 			xmlto 			      fop 
	@pip install pkgconfig mako Jinja2
	@pip install package_name setuptools --user

compiler_check:
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

checkfirst: compiler_check
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
	@cd libs; \
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
	@cd exts; \
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
	@cd exts; \
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
	@cd uix && \
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
	@cd uix; \
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
##
clean: checkfirst
	@for cat in $(COMPDIR); do                 \
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
	@for cat in $(COMPDIR); do                 \
		cd $$cat ;                             \
		for dir in $(SUBDIR); do               \
			[ ! -d $$dir ] || (                \
				[ ! -d $$dir/$(BUILDDIR) ] ||  \
					\rm -rf $$dir/$(BUILDDIR)/`cat $$dir/Makefile | grep "DIR=" | head -n 1 | sed "s/DIR=//"`/* ; \
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
	@make -C gnu/sources -f ../Makefile download
	@[ -d $(KERNDIR)/$(KERNELVER) ] || make -C $(BDDIR)/kernel prepare
	@for cat in $(COMPDIR); do                 \
		cd $$cat ;                             \
		for dir in $(SUBDIR); do               \
			[ ! -d $$dir ] || (                \
				[ -f $$dir/$(DNOUT) ] || touch $$dir/$(DNOUT)  ; \
			)                                  \
		done ;                                 \
		cd .. ;                                \
	done

download_epilogue:
	@for cat in $(COMPDIR); do                 \
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
			[ ! -f $$cdir/$$dir/$(DNOUT) ] || \rm -rf $$cdir/$$dir/$(DNOUT) ;   \
		done ;                                						            \
	done

##
## extdisk
## extdisk_clean
##
extdisk: compiler_check
	@cd gnu; make -f Makefile hdrpath=$(EXTINSTDIR) setup_headers
	@$(call DO_EXCL_SINGLE,\
	       $(TOPDIR)/scripts/setupdisk.sh build $(STAGEDIR) $(EXTDISK) $(BDDIR)/$(EXTDISKNM) $(EXTDISKBLKS) )
	@[ -z $(CFGDISKNM) ] || sudo dd if=/dev/zero of=$(EXT4CFG) bs=1M count=$(CFGDISKBLKS)
	@[ -z $(CFGDISKNM) ] || sudo mkfs.ext4 -j -F -L $(CFGVOLNM) $(EXT4CFG)


extdisk_clean: compiler_check 
	@$(call DO_EXCL_SINGLE, \
		$(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(EXTDISKNM))
	@[ -z $(CFGDISKNM) ] || ( [ ! -f $(BDDIR)/$(CFGDISKNM) ] || \rm -rf $(BDDIR)/$(CFGDISKNM) )
			

##
## uidisk
## uidisk_clean
##
uidisk: compiler_check 
	@[ "$(UIDISK)" = "" ] ||  ( \
		$(call DO_EXCL_SINGLE,\
			$(TOPDIR)/scripts/setupdisk.sh build $(BDDIR)/_uidir $(UIDISK) $(BDDIR)/$(UIDISKNM) $(UIDISKBLKS)) )

uidisk_clean: compiler_check 
	@[ "$(UIDISK)" = "" ] ||  ( \
		$(call DO_EXCL_SINGLE,\
			$(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(UIDISKNM) ) )
			

##
## BUILDUP_ROOTFS --> Please refer to boards/$(TBOARD)/env.mk 
##
run_bootstrap: checkfirst
	@[ -f $(BOARD_BUILDOUT) ] || touch $(BOARD_BUILDOUT)
	@make -C apps/busybox  destination=$(XBASEDIR) -f Makefile.bootstrap download prepare all install   | tee -a $(BOARD_BUILDOUT)
	@make -C apps/bash     destination=$(XBASEDIR) install                                              | tee -a $(BOARD_BUILDOUT)
	@$(shell $(BUILDUP_ROOTFS)) > /dev/null                                                             | tee -a $(BOARD_BUILDOUT)
	@[ ! -f $(XBASEDIR)/etc/init.d/rcS ]         || chmod ugo+x $(XBASEDIR)/etc/init.d/rcS
	@[ ! -f $(XBASEDIR)/etc/init.d/rc.shutdown ] || chmod ugo+x $(XBASEDIR)/etc/init.d/rc.shutdown

boot_%: checkfirst
	@echo ""
	@echo "Bootloader configuration/cleanup"
	@echo ""
	@make -C $(BDDIR)/boot isodir=$(ISODIR) $(subst boot_,,$@)

kernel_%: checkfirst
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

modules_%: checkfirst
	@if [ "$(subst mod_,,$@)" = "install" ] ; then \
		make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) install ;              \
		make -C $(BDDIR)        destination=$(INSTALLDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) install ; \
	fi

board: run_bootstrap
	@[ ! -f $(BOARD_BUILDOUT) ] || rm $(BOARD_BUILDOUT)
	@touch $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@echo "Preparing board ..."                                               | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@make -C $(BDDIR) destination=$(INSTALLDIR) isodir=$(ISODIR) prepare                    2>&1 | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@echo "Building kernel ..."                                               | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) dnfw prepare all    2>&1 | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@echo "External modules ..."                                              | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@make -C $(BDDIR)/kernel KERNDIR=$(KERNDIR) destination=$(INSTALLDIR) extmod            2>&1 | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@echo "Copying static rootfs ..."                                         | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@cp -rf $(BDDIR)/rootfs/* $(INSTALLDIR)/
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@echo "Collecting $(INSTALLDIR) and GLIB to $(FINDIR) ..."                | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@[ ! -d $(FINDIR) ] || \rm -rf $(FINDIR)
	@mkdir -p $(FINDIR)
	@cd $(XBASEDIR) ; find . -depth -print | cpio -pdm $(FINDIR)
	@make -C apps/glibc destination=$(FINDIR)/$(INSTSUBFN) install_glibc      2>&1 | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@echo "Setting for copying $(BOOTSTRAP_LIBS) and $(BOOTSTRAP_LDRS) ... "  | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                  | tee -a $(BOARD_BUILDOUT)
	@cd $(FINDIR)/$(INSTSUBFN)/lib ; \
		for fn in $(BOOTSTRAP_LIBS); do [ ! -f $$fn ] || cp -f $$fn ../../lib/ ; done
	@cd $(FINDIR); \
		[ -d $(dir $(BOOTSRTAP_LDRS)) ] || mkdir -p $(dir $(BOOTSTRAP_LDRS))
	@cd $(FINDIR)/$(INSTSUBFN); \
		[ -d $(dir $(BOOTSRTAP_LDRS)) ] || mkdir -p $(dir $(BOOTSTRAP_LDRS))
	@echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	@echo "Copying $(notdir $(BOOTSTRAP_LDRS)) to $(FINDIR)/$(INSTSUBFN)/lib64 ..."         | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	@[ -f $(FINDIR)/$(INSTSUBFN)/lib64/$(notdir $(BOOTSTRAP_LDRS)) ] || \
		cp -rf  $(INSTALLED_LDRS)  $(FINDIR)/$(INSTSUBFN)/lib64
	@echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	@echo "Copying $(notdir $(BOOTSTRAP_LDRS)) to $(FINDIR)/lib64 ..."                      | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	@[ -f $(FINDIR)/lib64/$(notdir $(BOOTSTRAP_LDRS)) ]              || \
		cp -rf  $(INSTALLED_LDRS)  $(FINDIR)/lib64
	@echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	@echo "Copying $(notdir $(BOOTSTRAP_LDRS)) to $(FINDIR)/lib ..."                        | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                                | tee -a $(BOARD_BUILDOUT)
	@[ -f $(FINDIR)/lib/$(notdir $(BOOTSTRAP_LDRS)) ]                || \
		cp -rf  $(INSTALLED_LDRS)  $(FINDIR)/lib

ramdisk: checkfirst
	@[ -f $(BOARD_BUILDOUT) ] || touch $(BOARD_BUILDOUT)
	@echo ""                                                                                2>&1 | tee -a $(BOARD_BUILDOUT)
	@echo "Compiling kernel and combining squash image ..."                                 2>&1 | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                                2>&1 | tee -a $(BOARD_BUILDOUT)
	@make -C $(BDDIR)/kernel destination=$(FINDIR) isodir=$(ISODIR) install                 2>&1 | tee -a $(BOARD_BUIDLOUT)
	@echo ""                                                                                2>&1 | tee -a $(BOARD_BUILDOUT)
	@echo "Build up image ..."                                                              2>&1 | tee -a $(BOARD_BUILDOUT)
	@echo ""                                                                                2>&1 | tee -a $(BOARD_BUILDOUT)
	@make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) install                         2>&1 | tee -a $(BOARD_BUILDOUT)

cleanup:
	@[ ! -d $(XBASEDIR)   ] || \rm -rf $(XBASEDIR)
	@[ ! -d $(STAGEDIR)   ] || \rm -rf $(STAGEDIR)
	@[ ! -d $(UIXINSTDIR) ] || \rm -rf $(UIXINSTDIR)
	@[ ! -d $(FINDIR)     ] || \rm -rf $(FINDIR)
	@[ ! -f $(BOARD_BUILDOUT)  ] || \rm -rf $(BOARD_BUILDOUT)
	@make -C $(BDDIR)        isodir=$(ISODIR) isoname=$(IMAGENAME) uninstall
	@make -C $(BDDIR)/kernel uninstall
	@cd $(BDDIR); \
		$(call DO_EXCL_SINGLE,\
			[ ! -f $(BDDIR)/$(EXTDISKNM) ] || $(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(EXTDISKNM) )
	@cd $(BDDIR); \
		$(call DO_EXCL_SINGLE,\
			[ ! -f $(BDDIR)/$(CFGDISKNM) ] || $(TOPDIR)/scripts/setupdisk.sh clean $(BDDIR)/$(CFGDISKNM) )

toolchain:
	@[ -f $(TOOLCHAIN_BUILDOUT) ] || touch $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "Preparing sources ... "                2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@$(call DO_EXCL_SINGLE,make -C gnu/sources -f ../Makefile download)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "BINUTILS ..."                          2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@[ -d gnu/binutils/build/$(_ARCH_)/$(BINUTILS) ] || mkdir -p gnu/binutils/build/$(_ARCH_)/$(BINUTILS)
	@cd gnu/binutils/build/$(_ARCH_)/$(BINUTILS); \
		$(call DO_EXCL_SINGLE, \
			make -f $(TOPDIR)/gnu/Makefile config_binutils 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)) && \
		make -f $(TOPDIR)/gnu/Makefile setup_binutils  2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "Kernel Headers ..."                    2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@[ -d $(KERNDIR)/$(KERNELVER) ] || \
		make -C $(BDDIR)/kernel destination=$(INSTALLDIR) isodir=$(ISODIR) prepare 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@cd gnu; \
		$(call DO_EXCL_SINGLE, \
			make -f Makefile hdrpath=$(TOOLCHAIN_ROOT)/$(PLATFORM) setup_headers 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT))
	@[ ! -f $(TOOLCHAIN_ROOT)/$(PLATFORM)/include/asm/.install ] || ( \
		echo ""  ;                     \
		echo "Cleaning up kernel folder ..." ; \
		echo ""  ;                     \
		\rm -rf $(KERNDIR)/$(KERNELVER) ; )
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GCC Bootstrap ..."                     2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@[ -d gnu/gcc/build/$(_ARCH_)/$(GCC) ] || mkdir -p gnu/gcc/build/$(_ARCH_)/$(GCC)
	@cd gnu/gcc/build/$(_ARCH_)/$(GCC); \
		$(call DO_EXCL_SINGLE, \
			make -f $(TOPDIR)/gnu/Makefile config_gcc 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)) && \
		make -f $(TOPDIR)/gnu/Makefile setup_gcc  2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GLIBC Bootstrap ..."                   2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@[ -d gnu/glibc/build/$(_ARCH_)/$(GLIBC) ] || mkdir -p gnu/glibc/build/$(_ARCH_)/$(GLIBC)
	@cd gnu/glibc/build/$(_ARCH_)/$(GLIBC); \
		$(call DO_EXCL_SINGLE, \
			make -f $(TOPDIR)/gnu/Makefile config_glibc 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)) && \
		make -f $(TOPDIR)/gnu/Makefile setup_glibc      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GCC Host Compiler ..."                 2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@cd gnu/gcc/build/$(_ARCH_)/$(GCC); \
		make -f $(TOPDIR)/gnu/Makefile setup_gcc_2     2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GLIBC Host Library ..."                2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@cd gnu/glibc/build/$(_ARCH_)/$(GLIBC); \
		make -f $(TOPDIR)/gnu/Makefile setup_glibc_2   2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo "GCC Extra Libraries ..."               2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@echo ""                                      2>&1 | tee -a $(TOOLCHAIN_BUILDOUT)
	@cd gnu/gcc/build/$(_ARCH_)/$(GCC); \
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

vm%:
	@if [ "$(subst vm,,$@)" = "run" ]; then                                \
		make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) run ;       \
	elif [ "$(subst vm,,$@)" = "stop" ]; then                              \
		make -C $(BDDIR) isodir=$(ISODIR) isoname=$(IMAGENAME) stop ;      \
	else                                                                   \
		echo "Only two commands; vmrun vmstop " ;                          \
	fi

pkglist:
	@for dir in $(INSTALLDIR) $(EXTINSTDIR) $(UIXINSTDIR) ; do 	\
		for subdir in $(LIBSSUBDIR) ; do 			            \
	    		[ ! -d $$dir$$subdir/pkgconfig ] || 		    \
				find $$dir$$subdir -name "*.pc" ; 	            \
			done ;						                        \
		done

folders:
	@echo $(SUBDIR)
