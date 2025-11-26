DIR=12.x
LOC=https://github.com/llvm/llvm-project/archive/refs/heads/release

## Directory name 
LLVMDN=llvm-project-release-$(DIR)

## 
## Default target
DEFTARGET=x86_64-any-linux-gnu

##
## Architecture Name 
ifeq ($(_CORE_),aarch64)
	ARCHNAME=AArch64
	TARGETS="$(ARCHNAME);X86"
endif
ifeq ($(_CORE_),x86_64)
	ARCHNAME=X86
	TARGETS="X86"
endif

##
## Source folders
SRCDIR=../sources

##
## Host tools required 
HOSTBUILD=host/build
LLVM_TBLGEN_FILE=$(HOSTBUILD)/bin/llvm-tblgen
CLANG_TBLGEN=bin/clang-tblgen
CLANG_TBLGEN_FILE=$(HOSTBUILD)/$(CLANG_TBLGEN)

##
## patch/patch for especially libc 
CLANG_PROJECTS="clang;lld;libc;libcxx;libcxxabi"

notice:
	@echo ""
	@echo ""
	@echo "    YOU BETTER USE +16G MAIN MEMORY !!! "
	@echo ""
	@echo ""
	@echo "    FOR POOR PROGRAMMER LIKE ME - 8G DRAM + Intel i5 Dell Vostro Laptop  --;  "
	@echo ""
	@echo "    # sudo dd if=/dev/zero of=/media/todd/disk/swapdisk.img bs=128M count=64 --- 8G swap file creation           "
	@echo "    # sudo chmod 0600 /media/todd/disk/swapdisk.img                          --- Swap disk properties            "
	@echo "    # sudo mkswap /media/todd/disk/swapdisk.img                              --- Making it swap partition format "
	@echo "    # sudo swapon /media/todd/disk/swapdisk.img                              --- Adding it swappable space       "
	@echo "    # sudo swapon              " 
	@echo "    [sudo] password for todd:  "
	@echo "    NAME                          TYPE      SIZE   USED PRIO "
	@echo "    /dev/dm-1                     partition 976M 884.8M   -2 "
	@echo "    /media/todd/disk/swapdisk.img file        8G 737.1M   -3 "
	@echo "    #                          "
	@echo ""
	@echo "    YOU HAVE TO PATIENTLY WAIT FOR THE COMPLETION OF COMPILATION !! "
	@echo "    DO NOT BOTHER YOUR LAPTOP'S HARD WORK !!  JUST WAIT !!          "
	@echo ""
	@echo ""

prepare: 
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/host/build   ] || mkdir -p $(BUILDDIR)/host/build
	@[ -f $(SRCDIR)/$(DIR).zip  ] || wget --no-check-certificate -P $(SRCDIR) $(LOC)/$(DIR).zip
	@[ -d $(BUILDDIR)/$(LLVMDN) ] || ( \
		$(UNCOMPRESS) $(SRCDIR)/$(DIR).zip $(BUILDDIR) ;                 \
		[ ! -f patch/patch ] ||     (                                    \
			cd $(BUILDDIR)/$(LLVMDN) ;                               \
			cat ../../../patch/patch | patch -p 1 ;                  \
		) )
	@[ ! -d $(BUILDDIR)/host/build ] || (                                    \
		cd $(BUILDDIR)/host/build  ;                                     \
		cmake -G Ninja                                                   \
			-D LLVM_PARALLEL_LINK_JOBS=4                             \
			-D CMAKE_BUILD_TYPE="Release"                            \
			-D CMAKE_INSTALL_PREFIX=$(TOOLCHAIN_ROOT)                \
			-D BUILD_SHARED_LIBS=ON                                  \
			-D LLVM_DEFAULT_TARGET_TRIPLE=$(DEFTARGET)               \
			-D LLVM_TARGETS_TO_BUILD=$(TARGETS)                      \
			-D LLVM_ENABLE_PROJECTS=$(CLANG_PROJECTS)                \
			-D LLVM_ENABLE_RTTI=ON                                   \
			-D LLVM_ENABLE_EH=ON                                     \
			-D LLVM_LIBC_ENABLE_LINTING=OFF                          \
			-D LLVM_ENABLE_WARNINGS=OFF                              \
			-D LLVM_ENABLE_WERROR=OFF                                \
			-D LLVM_ENABLE_PEDANTIC=OFF                              \
			-D LLVM_ENABLE_ASSERTIONS=OFF                            \
			-D LLVM_INCLUDE_EXAMPLES=OFF                             \
			-D LLVM_INCLUDE_TESTS=OFF                                \
			-D LLVM_INCLUDE_DOCS=OFF                                 \
			-D CLANG_BUILD_EXAMPLES=OFF                              \
			-D CLANG_ENABLE_ARCMT=OFF                                \
			-D CLANG_ENABLE_STATIC_ANALYZER=OFF                      \
			../../$(LLVMDN)/llvm ;                                   \
	)

all clean install: notice
	@[ ! -d $(BUILDDIR)/host/build ] || (                                \
		cd $(BUILDDIR)/host/build  ;                                     \
		ninja -v  $@ ;                                                   \
		[ "$@" != "install" ] || (                                       \
			[ -f $(TOOLCHAIN_ROOT)/$(CLANG_TBLGEN) ] ||              \
				cp -rf $(CLANG_TBLGEN) $(TOOLCHAIN_ROOT)/$(CLANG_TBLGEN) )  \
	)

