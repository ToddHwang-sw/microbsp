DIR=git-2.24.2
LOC=https://mirrors.edge.kernel.org/pub/software/scm/git/

include $(TOPDIR)/scripts/extenv.mk

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

MICB_CONFIGURE_PRG = ./configure 

MICB_CONFIGURE_MAKEOPTS += "V=1"

MICB_DEPENDS = openssl libssh2 nettle zlib expat iconv libcurl

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || wget -P $(MICBSRC) $(LOC)/$(DIR).tar.gz
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		tar zxvf $(MICBSRC)/$(DIR).tar.gz -C $(BUILDDIR) > /dev/null ; \
		cd $(BUILDDIR)/$(DIR); \
		[ ! -f ../../../$(MICB_PATCH_FN) ] || cat ../../../$(MICB_PATCH_FN) | patch -p1 )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


