DIR=12.x
LOC=https://github.com/llvm/llvm-project/archive/refs/heads/release

## Target folder 
LLVMTRGTLOC=uix/llvm

## Host folder 
LLVMHLOC=$(TOOLCHAIN_ROOT)

## Source folder 
LLVMDN=llvm-project-release-$(DIR)

##
## FLAGS ...
include $(TOPDIR)/arch/$(_ARCH_)/env.mk
CROSS_USER_CFLAGS += -DMB_LEN_MAX=16 -DULLONG_MAX=18446744073709551615ULL -DPATH_MAX=4096 -DNAME_MAX=255 -D_POSIX_ARG_MAX=_SC_ARG_MAX
CROSS_USER_LFLAGS += -lpthread

##
## Architecture Name 
ifeq ($(_CORE_),aarch64)
	DEFARCH=AArch64
	ARCHNAME=$(DEFARCH)
endif
ifeq ($(_CORE_),x86_64)
	DEFARCH=X86
	ARCHNAME=$(DEFARCH)
endif

_SYSROOT=$(subst install: ,,$(shell $(CROSS_COMP_PREFIX)gcc --print-search-dirs | grep install))

##
## VERY IMPORTANT !! 
##
LLVM_TBLGEN_FILE=bin/llvm-tblgen
CLANG_TBLGEN_FILE=bin/clang-tblgen

##
## Projects
CLANG_PROJECTS="llvm;libcxx;libcxxabi"

notice:
	@echo "-----------------------------------------------------------------------------------------------------------------------------"
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
	@echo "-----------------------------------------------------------------------------------------------------------------------------"

prepare: 
	[ -f $(TOPDIR)/gnu/sources/$(DIR).zip ] || exit 1
	[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	[ -d $(BUILDDIR)/target/install ] || mkdir -p $(BUILDDIR)/target/install
	[ -d $(BUILDDIR)/target/build   ] || mkdir -p $(BUILDDIR)/target/build
	[ -d $(BUILDDIR)/target/$(LLVMDN) ] || $(UNCOMPRESS) $(TOPDIR)/gnu/sources/$(DIR).zip $(BUILDDIR)/target
	[ ! -d $(BUILDDIR)/target/build ] || (  \
			cd $(BUILDDIR)/target/build ;               \
			cmake -G "Unix Makefiles"                   \
				-D CMAKE_BUILD_TYPE="Release"           \
				-D CMAKE_INSTALL_PREFIX=""              \
				-D CMAKE_CROSSCOMPILING=True            \
				-D CMAKE_C_COMPILER="$(CROSS_COMP_PREFIX)gcc"   \
				-D CMAKE_CXX_COMPILER="$(CROSS_COMP_PREFIX)g++" \
				-D CMAKE_ASM_COMPILER="$(CROSS_COMP_PREFIX)as"  \
				-D LLVM_LIBC_ENABLE_LINTING=OFF                 \
				-D LLVM_DEFAULT_TARGET_TRIPLE=$(PLATFORM)       \
				-D LLVM_ENABLE_PIC=True                         \
				-D LLVM_TABLEGEN=$(LLVMHLOC)/$(LLVM_TBLGEN_FILE)         \
				-D CLANG_TABLEGEN=$(LLVMHLOC)/$(CLANG_TBLGEN_FILE)       \
				-D LLVM_TARGET_ARCH=$(DEFARCH)                           \
				-D LLVM_TARGETS_TO_BUILD=$(ARCHNAME)                     \
				-D LLVM_PARALLEL_LINK_JOBS=4                             \
				-D LLVM_PTHREAD_LIB="-lpthread"                          \
				-D ZLIB_INCLUDE_DIR=$(INSTALLDIR)/include                \
				-D ZLIB_LIBRARY=$(INSTALLDIR)/lib                        \
				-D LIBXML2_INCLUDE_DIR=$(INSTALLDIR)/include             \
				-D LIBXML2_LIBRARY=$(INSTALLDIR)/lib                     \
				-D FFI_INCLUDE_DIR=$(INSTALLDIR)/include                 \
				-D FFI_LIBRARY=$(INSTALLDIR)/lib                         \
				-D OPTIMIZE_FOR_NATIVE=OFF                               \
				-D COMPILER_SUPPORTS_MARCH_NATIVE=OFF                    \
				-D BUILD_SHARED_LIBS=ON                                  \
				-D CMAKE_SYSROOT="$(_SYSROOT)"                           \
				-D CMAKE_FIND_ROOT_PATH="$(_SYSROOT)"                    \
				-D CMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER               \
				-D CMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY                \
				-D CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY                \
				-D LLVM_ENABLE_PROJECTS=$(CLANG_PROJECTS)                \
				-D CMAKE_C_FLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)"          \
				-D CMAKE_CXX_FLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)"        \
				-D CMAKE_EXE_LINKER_FLAGS="$(CROSS_COMP_FLAGS) $(CROSS_USER_LFLAGS)" \
				-D LLVM_ENABLE_RTTI=ON                                   \
				-D LLVM_ENABLE_EH=ON                                     \
				-D LLVM_ENABLE_WARNINGS=OFF                              \
				-D LLVM_ENABLE_WERROR=OFF                                \
				-D LLVM_ENABLE_PEDANTIC=OFF                              \
				-D LLVM_ENABLE_ASSERTIONS=OFF                            \
				-D LLVM_INCLUDE_EXAMPLES=OFF                             \
				-D LLVM_INCLUDE_TESTS=OFF                                \
				-D LLVM_INCLUDE_DOCS=OFF                                 \
				-D LLVM_BUILD_BENCHMARKS=OFF                             \
				-D LLVM_INCLUDE_BENCHMARKS=OFF                           \
				-D CLANG_BUILD_EXAMPLES=OFF                              \
				-D CLANG_ENABLE_ARCMT=OFF                                \
				-D CLANG_ENABLE_STATIC_ANALYZER=OFF                      \
				../$(LLVMDN)/llvm )

all install uninstall clean: notice 
	[ ! -d $(BUILDDIR)/target/build ] || (            \
		cd $(BUILDDIR)/target/build ;                 \
		if [ "$@" == "all" ]; then                    \
			make DESTDIR=$(TOPDIR)/$(LLVMTRGTLOC)/$(BUILDDIR)/target/install VERBOSE=1 ;      \
		else                                                                              \
			make DESTDIR=$(TOPDIR)/$(LLVMTRGTLOC)/$(BUILDDIR)/target/install VERBOSE=1 $@ ;   \
		fi )

