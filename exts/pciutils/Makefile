DIR=pciutils
LOC=https://github.com/pciutils

include $(TOPDIR)/scripts/extenv.mk

ifeq ($(_CORE_),aarch64)
	HOSTINFO="AARCH64-LINUX"
endif
ifeq ($(_CORE_),x86_64)
	HOSTINFO="X86_64-LINUX"
endif
ifeq ($(_CORE_),arm)
	HOSTINFO="ARM-LINUX"
endif
ifeq ($(_CORE_),riscv64)
	HOSTINFO="RISCV64-LINUX"
endif

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)


all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
         cd $(BUILDDIR)/$(DIR); \
		 make ZLIB=no CROSS_COMPILE=$(CROSS_COMP_PREFIX) HOST=$(HOSTINFO) TARGET_ARCH="$(CROSS_COMP_FLAGS) $(CROSS_USER_CFLAGS)" \
			$(MICB_CONFIGURE_MAKEOPTS) $@ )

