DIR=$(GCC)

GCCDIR=$(TOPDIR)/gnu/sources/
GCC_TARGET_FILE=$(DIR)-$(PLATFORM).tar.gz

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_OPTS += \
	--disable-compressed-debug-sections \
	--enable-libstdcxx \
	--disable-libsanitizer \
	--disable-multilib \
	--disable-nls \
	--enable-languages=$(SUPPORTED_LANGUAGES) \
	--with-gmp=$(INSTALLDIR) \
	--with-mpfr=$(INSTALLDIR) \
	--with-mpc=$(INSTALLDIR) 

MICB_DEPENDS = mpc mpfr gmp

download:
	@echo "Downloading $(GCC) - completed"

prepare: 
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( \
		tar zxvf $(GCCDIR)/$(DIR).tar.gz -C $(MICBSRC); \
		cd $(MICBSRC)/$(DIR) ; \
		cat ../../patch/patch_$(GCC) | patch -p 1 )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS) 
	
all clean:
	@[ -f $(BUILDDIR)/$(GCC_TARGET_FILE) ] || ( \
		[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
			cd $(BUILDDIR)/$(DIR); \
			make -j8 CC_FOR_BUILD=gcc CXX_FOR_BUILD=g++ $@ ) \
	)

install:
	@[ -f $(BUILDDIR)/$(GCC_TARGET_FILE) ] || ( \
		[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
			cd $(BUILDDIR)/$(DIR); \
			make DESTDIR=`pwd`/../target install;  \
			cd ../target; \
			tar zcvf ../$(GCC_TARGET_FILE) * > /dev/null ) \
	)
	@tar zxvf $(BUILDDIR)/$(GCC_TARGET_FILE) -C $(MICB_INSTALL_PLACE)

uninstall:

