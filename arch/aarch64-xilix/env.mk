CROSS_COMP_PREFIX=$(PLATFORM)-gnu-
CROSS_COMP_FLAGS=-mlittle-endian -mabi=lp64 -mpc-relative-literal-loads -march=armv8-a -mtune=cortex-a53

CC=$(CROSS_COMP_PREFIX)gcc
CXX=$(CROSS_COMP_PREFIX)g++
LD=$(CROSS_COMP_PREFIX)ld
AR=$(CROSS_COMP_PREFIX)ar
OBJCOPY=$(CROSS_COMP_PREFIX)objcopy
RANLIB=$(CROSS_COMP_PREFIX)ranlib
STRIP=$(CROSS_COMP_PREFIX)strip

##
## Selected library between GNU Library and UClibc
##
export LIB=glibc

##
## Libs required for bootstrap & Languages 
## 
export LLOADER_NAME=linux-aarch64
export CROSS_COMP_FLAGS+= 
export BOOTSTRAP_LIBS=\
		ld-2.30.so   ld-$(LLOADER_NAME).so.* \
		libc.so*     libc-2.30.so \
		libdl.so*    libdl-2.30.so \
		libm.so*     libm-2.30.so \
		libcrypt.so* libcrypt-2.30.so \
		libtinfo.so* libtinfo.so.5.9
export SUPPORTED_LANGUAGES=c,c++,go

export CROSS_COMP_PREFIX
export CROSS_COMP_FLAGS

## Linux Kernel compile
export KERNELMAKE=make ARCH=arm64 CROSS_COMPILE=$(CROSS_COMP_PREFIX) CFLAGS_KERNEL="$(CROSS_COMP_FLAGS)" CFLAGS_MODULE="$(CROSS_COMP_FLAGS)"
