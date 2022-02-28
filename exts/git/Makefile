DIR=git-2.24.2
LOC=https://mirrors.edge.kernel.org/pub/software/scm/git/

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	ac_cv_fread_reads_directories=yes \
	ac_cv_snprintf_returns_bogus=yes

MICB_CONFIGURE_OPTS += \
	--with-curl=$(INSTALLDIR) 	\
	--with-expat=$(INSTALLDIR) 	\
	--with-openssl=$(INSTALLDIR) 	\
	--with-libpcre2=$(INSTALLDIR) 	

MICB_CONFIGURE_PRG = ./configure 

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES) 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


