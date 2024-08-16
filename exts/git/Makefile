DIR=git-2.24.2
LOC=https://mirrors.edge.kernel.org/pub/software/scm/git/

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	ac_cv_fread_reads_directories=yes \
	ac_cv_snprintf_returns_bogus=yes

MICB_CONFIGURE_PRG = ./configure 

MICB_DEPENDS = openssl libssh2 libtasn1 nettle zlib expat iconv libcurl libpcre2

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz ; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


