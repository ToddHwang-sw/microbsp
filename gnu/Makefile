RP=../../..

ifeq ($(_CORE_),x86_64)
	KARCH=x86_64
endif
ifeq ($(_CORE_),aarch64)
	KARCH=arm64
endif
ifeq ($(_CORE_),arm)
	KARCH=arm
endif

download:
	@[ -f $(BINUTILS).tar.bz2 ] || wget https://ftp.gnu.org/pub/gnu/binutils/$(BINUTILS).tar.bz2
	@[ -f $(GCC).tar.gz       ] || wget https://ftp.gnu.org/pub/gnu/gcc/$(GCC)/$(GCC).tar.gz
	@[ -f $(GLIBC).tar.gz     ] || wget https://ftp.gnu.org/pub/gnu/glibc/$(GLIBC).tar.gz

config_binutils:
	@[ -d $(RP)/$(BINUTILS) ] || tar jxvf $(RP)/../sources/$(BINUTILS).tar.bz2 -C $(RP)
	$(RP)/$(BINUTILS)/configure --prefix=$(TOOLCHAIN_ROOT) --program-prefix=$(PLATFORM)- --target=$(PLATFORM) --build=$(PLATFORM) \
			--without-gmp --without-mpfr --without-mpc --without-isl --disable-multilib

setup_binutils:
	@make
	@make install

setup_headers:
	@make ARCH=$(KARCH) INSTALL_HDR_PATH=$(hdrpath) -C $(KERNDIR)/$(KERNELVER) headers_install

config_gcc:
	[ -d $(RP)/$(GCC) ] || tar zxvf $(RP)/../sources/$(GCC).tar.gz -C $(RP)
	cd $(RP)/$(GCC);  \
		[ -f PATCHED ] || cat ../patch/gcc.host.patch_$(GCC) | patch -p1
	AR=ar \
	$(RP)/$(GCC)/configure --prefix=$(TOOLCHAIN_ROOT) -target=$(PLATFORM) \
			--disable-compressed-debug-sections \
			--disable-multiarch \
			--enable-libstdcxx \
			--enable-languages=$(SUPPORTED_LANGUAGES) \
			--without-gmp --without-mpfr --without-mpc --without-isl --disable-multilib

setup_gcc:
	make all-gcc
	make install-gcc

setup_gcc_2:
	make 	GCC_FOR_TARGET="$(TOOLCHAIN_ROOT)/bin/$(CROSS_COMP_PREFIX)gcc $(CROSS_COMP_FLAGS)" \
		NM_FOR_TARGET=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/nm \
		AR_FOR_TARGET=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/ar \
		AS_FOR_TARGET=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/as \
		STRIP_FOR_TARGET=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/strip \
		RANLIB_FOR_TARGET=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/ranlib \
		OBJCOPY_FOR_TARGET=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/objcopy \
		OBJDUMP_FOR_TARGET=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/objdump \
			all-target-libgcc install-target-libgcc

setup_gcc_3:
	make 	CC="$(TOOLCHAIN_ROOT)/bin/$(CROSS_COMP_PREFIX)gcc $(CROSS_COMP_FLAGS)" \
		NM=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/nm \
		LD=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/ld \
		AR=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/ar \
		AS=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/as \
		LD=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/ld \
		NM=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/nm \
		STRIP=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/strip \
		RANLIB=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/ranlib \
		OBJCOPY=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/objcopy \
		OBJDUMP=$(TOOLCHAIN_ROOT)/$(PLATFORM)/bin/objdump \
			all-target-libstdc++-v3 install-target-libstdc++-v3 \
			all-target-libssp install-target-libssp

#
#
#     G N U    L i b r a r y 
#
#

config_glibc:
	[ -d $(RP)/$(GLIBC) ] || tar zxvf $(RP)/../sources/$(GLIBC).tar.gz -C $(RP)
	cd $(RP)/$(GLIBC); \
		[ -f PATCHED ] || cat ../patch/glibc.patch_$(GLIBC) | patch -p1
	CC="$(TOOLCHAIN_ROOT)/bin/$(CROSS_COMP_PREFIX)gcc $(CROSS_COMP_FLAGS)" \
	CXX="$(TOOLCHAIN_ROOT)/bin/$(CROSS_COMP_PREFIX)g++ $(CROSS_COMP_FLAGS)" \
	lib_cv_forced_unwind=yes \
		$(RP)/$(GLIBC)/configure --prefix=$(TOOLCHAIN_ROOT)/$(PLATFORM) \
			--build=$(HOSTSYSTEM) --host=$(PLATFORM) --target=$(PLATFORM) \
			--with-headers=$(TOOLCHAIN_ROOT)/$(PLATFORM)/include \
			--disable-multi-arch $(GLIBC_ARCH_OPTIONS)

setup_glibc:
	make CC="$(TOOLCHAIN_ROOT)/bin/$(CROSS_COMP_PREFIX)gcc $(CROSS_COMP_FLAGS)" install-bootstrap-headers=yes install-headers
	make CC="$(TOOLCHAIN_ROOT)/bin/$(CROSS_COMP_PREFIX)gcc $(CROSS_COMP_FLAGS)" csu/subdir_lib
	install csu/crt1.o csu/crti.o csu/crtn.o $(TOOLCHAIN_ROOT)/$(PLATFORM)/lib
	$(TOOLCHAIN_ROOT)/bin/$(CROSS_COMP_PREFIX)gcc $(CROSS_COMP_FLAGS) -nostdlib -nostartfiles -shared -x c /dev/null -o $(TOOLCHAIN_ROOT)/$(PLATFORM)/lib/libc.so
	touch $(TOOLCHAIN_ROOT)/$(PLATFORM)/include/gnu/stubs.h

setup_glibc_2:
	make CC="$(TOOLCHAIN_ROOT)/bin/$(CROSS_COMP_PREFIX)gcc $(CROSS_COMP_FLAGS)"
	make CC="$(TOOLCHAIN_ROOT)/bin/$(CROSS_COMP_PREFIX)gcc $(CROSS_COMP_FLAGS)" install
