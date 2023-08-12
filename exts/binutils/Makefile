DIR=$(BINUTILS)

BINUTILSDIR=$(TOPDIR)/gnu/sources
BINUTILS_TARGET_FILE=$(BINUTILS)-$(PLATFORM).tar.gz

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_OPTS += --disable-multilib \
	--with-mpc=$(INSTALLDIR)   \
	--with-mpfr=$(INSTALLDIR)  \
	--with-gmp=$(INSTALLDIR)   \

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( tar jxvf $(BINUTILSDIR)/$(BINUTILS).tar.bz2 -C $(MICBSRC) ; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( 		\
                cd $(BUILDDIR)/$(DIR); 			\
		make $(MICB_CONFIGURE_MAKEOPTS) $@ ; 	\
		[ ! -f $(BUILDDIR)/$(BINUTILS_TARGET_FILE) ] || rm $(BUILDDIR)/$(BINUTILS_TARGET_FILE) \
	)

install:
	@[ -f $(BUILDDIR)/$(BINUTILS_TARGET_FILE) ] || ( 			\
		[ ! -d $(BUILDDIR)/$(DIR) ] || ( 				\
                	cd $(BUILDDIR)/$(DIR); 					\
			make DESTDIR=`pwd`/../target install;  			\
			cd ../target; 						\
			tar zcvf ../$(BINUTILS_TARGET_FILE) * > /dev/null ) 	\
	)
	@tar zxvf $(BUILDDIR)/$(BINUTILS_TARGET_FILE) -C $(MICB_INSTALL_PLACE)

uninstall:
