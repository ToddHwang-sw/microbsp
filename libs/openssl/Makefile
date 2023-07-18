DIR=openssl
LOC=-b openssl-3.1 https://github.com/openssl

include $(TOPDIR)/scripts/env.mk

ifeq ($(_CORE_),arm)
	_ARCH=armv4
else
	_ARCH=$(_CORE_)
endif

MICB_CONFIGURE_CMD=../../../$(MICBSRC)/$(DIR)/Configure 
MICB_CONFIGURE_OPTS=--prefix=/ --openssldir=/etc/openssl shared no-async linux-$(_ARCH)
MICB_CONFIGURE_MAKEOPTS += $(COMP_CMDS)

prepare: 
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )
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

