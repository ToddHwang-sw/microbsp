DIR=postgresql-11.22
LOC=https://ftp.postgresql.org/pub/source/v11.22

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_OPTS +=\
	--with-readline   \
	--with-zlib       \
	--with-pam        \
	--with-openssl    \
	--with-python     \
	--with-libxml     \
	--without-perl    \
	--without-llvm    \
	--without-selinux \
	--without-systemd

MICB_CONFIGURE_PRG = ./configure 

MICB_DEPENDS = readline zlib pam openssl python-$(PYTHON_SYSVER) libxml-2.0 libffi

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.bz2 ; $(MICB_PATCH) )

prepare: 
	@$(MICB_DUP_SOURCES)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); \
				make -C src/backend/ generated-headers; \
				cd src/backend; \
				make -C catalog distprep generated-header-symlinks; \
				cd ../../; \
				make $(MICB_CONFIGURE_MAKEOPTS) $@ )

install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

