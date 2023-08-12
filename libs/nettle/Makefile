DIR=nettle
LOC=https://git.lysator.liu.se/nettle

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_OPTS += \
	--enable-pic \
	--enable-openssl \
	--enable-public-key \
	--enable-mini-gmp \
	--disable-documentation

ifeq ($(_CORE_),aarch64)
	MICB_CONFIGURE_OPTS += \
		--enable-arm-neon \
		--enable-arm64-crypto \
		--disable-x86-aesni \
		--disable-x86-sha-ni 
endif
ifeq ($(_CORE_),x86_64)
	MICB_CONFIGURE_OPTS += \
		--disable-arm-neon \
		--disable-arm64-crypto \
		--enable-x86-aesni \
		--enable-x86-sha-ni 
endif

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare: 
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

