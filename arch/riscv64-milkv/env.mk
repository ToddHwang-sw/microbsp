export CROSS_COMP_PREFIX=$(PLATFORM)-
export CROSS_COMP_FLAGS=-mabi=lp64d -march=rv64imafdcv0p7xtheadp2 -mcmodel=medany -mstrict-align -mno-save-restore

CC=$(CROSS_COMP_PREFIX)gcc
CXX=$(CROSS_COMP_PREFIX)g++
LD=$(CROSS_COMP_PREFIX)ld
AR=$(CROSS_COMP_PREFIX)ar
OBJCOPY=$(CROSS_COMP_PREFIX)objcopy
RANLIB=$(CROSS_COMP_PREFIX)ranlib
STRIP=$(CROSS_COMP_PREFIX)strip
READELF=$(CROSS_COMP_PREFIX)readelf

##
## User Flags...
##
export CROSS_USER_CFLAGS=-I$(INSTALLDIR)/include
export CROSS_USER_LFLAGS=-L$(INSTALLDIR)/lib

##
## Compilation rule...
##
COMP_CMDS=CC="$(CC) $(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)" \
		CXX="$(CXX) $(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)" \
	  	LDFLAGS="$(CROSS_USER_LFLAGS)" \
	  	LD="$(LD)" \
		OBJCOPY="$(OBJCOPY)" \
		AR="$(AR)" \
		READELF="$(READELF)" \
		RANLIB="$(RANLIB)" \
		STRIP="$(STRIP)"
export COMP_CMDS

export SUPPORTED_LANGUAGES=c,c++

##
## GNU toolchains  - component information
##
export BINUTILS=binutils-2.42
export GCC=gcc-13.2.0
export GLIBC_VER=2.38
export GLIBC=glibc-$(GLIBC_VER)
export GLIBC_ARCH_OPTIONS=--enable-crypt
export GCC_ARCH_OPTIONS=
export BINUTILS_ARCH_OPTIONS=
