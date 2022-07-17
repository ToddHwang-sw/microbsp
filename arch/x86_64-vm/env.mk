export CROSS_COMP_PREFIX=$(PLATFORM)-
export CROSS_COMP_FLAGS=-march=x86-64 -m64 -falign-jumps=1 -falign-loops=1 -mtune=generic -mno-red-zone -mcmodel=large

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

##
## Libs required for bootstrap  
## 
export BOOTSTRAP_LIBS=\
		ld-2.30.so   ld-linux-x86-64.so.* \
		libc.so*     libc-2.30.so \
		libm.so*     libm-2.30.so \
		libcrypt.so* libcrypt-2.30.so
export SUPPORTED_LANGUAGES=c,c++
