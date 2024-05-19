DIR=openssl
LOC=-b openssl-3.1 https://github.com/openssl

include $(TOPDIR)/scripts/env.mk

_TARGET=linux-$(_CORE_)
ifeq ($(_CORE_),arm)
	_TARGET=linux-armv4
endif
ifeq ($(_CORE_),riscv64)
	_TARGET=linux64-riscv64
endif

MICB_CONFIGURE_CMD=../../../$(MICBSRC)/$(DIR)/Configure 
MICB_CONFIGURE_OPTS=--prefix=/ --openssldir=/etc/openssl shared no-async $(_TARGET)
MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS)

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare: 
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) )

install:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) install_sw )

clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) clean )

uninstall:

