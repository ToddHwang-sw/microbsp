DIR=openssh-portable
LOC=-b V_9_1 https://github.com/openssh

include $(TOPDIR)/scripts/extenv.mk

CROSS_USER_CFLAGS += -DMB_LEN_MAX=16

MICB_CONFIGURE_OPTS += --disable-strip --with-pam

MICB_CONFIGURE_RUNENV += \
		ac_cv_func_setlogin=no 

MICB_DEPENDS = zlib openssl libpam

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ "$@" == "install" ] || ( \
		@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ ) \
	)
	@[ "$@" != "install" ] || ( \
		[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); \
				make $(MICB_CONFIGURE_MAKEOPTS) install-mbsp ) \
    )


