DIR=git-2.24.2
LOC=https://mirrors.edge.kernel.org/pub/software/scm/git/

include $(TOPDIR)/scripts/extenv.mk

GIT_EXTRA_LIBS="-lnettle -lidn2 -lssh2 -lgnutls -lssl -lcrypto -lz -lcurl"

MICB_CONFIGURE_RUNENV += \
	ac_cv_fread_reads_directories=yes \
	ac_cv_snprintf_returns_bogus=yes

MICB_CONFIGURE_OPTS += \
	--with-zlib=$(INSTALLDIR) 	\
	--with-expat=$(INSTALLDIR) 	\
	--with-openssl=$(INSTALLDIR) 	\
	--with-libpcre2=$(INSTALLDIR) 	\
	--with-curl=$(EXTINSTDIR)	\
	--with-iconv=$(EXTINSTDIR)

MICB_CONFIGURE_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH) \
	LIBS=$(GIT_EXTRA_LIBS)

MICB_CONFIGURE_PRG = ./configure 

MICB_CONFIGURE_MAKEOPTS += 	\
	"V=1" 			\
	ENV_LIBS=$(GIT_EXTRA_LIBS)

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ ! -d $(BUILDDIR) ] || ( \
		wget -P $(MICBSRC) $(LOC)/$(DIR).tar.gz; \
		tar zxvf $(MICBSRC)/$(DIR).tar.gz -C $(BUILDDIR) > /dev/null ; \
		cd $(BUILDDIR)/$(DIR); \
		[ ! -f ../../../$(MICB_PATCH_FN) ] || cat ../../../$(MICB_PATCH_FN) | patch -p1 )
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS) )

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


