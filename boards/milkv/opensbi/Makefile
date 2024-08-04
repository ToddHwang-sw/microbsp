DIR=opensbi
LOC=https://github.com/riscv-software-src

include $(TOPDIR)/scripts/env.mk

##
## -mabi=lp64 -march=rv64imac_zicsr_zifencei -mcmodel=medany
##
MICB_CONFIGURE_MAKEOPTS += \
	PLATFORM=generic \
	CROSS_COMPILE=$(CROSS_COMP_PREFIX) \
	PLATFORM_RISCV_XLEN=64 \
	PLATFORM_RISCV_ABI=lp64 \
	PLATFORM_RISCV_ISA=rv64imac_zicsr_zifencei \
	PLATFORM_RISCV_CODE_MODEL=medany \

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)

prepare:

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
			cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

config:
	@echo "$(DIR) doesn't support config."

