#DIR=ipv6calc-2.2.0
#LOC=https://ftp.bieringer.de/pub/linux/IPv6/ipv6calc/
DIR=ipv6calc
LOC=https://github.com/pbiering
	
include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_RUNENV += \
	ac_cv_func_memcmp_working=yes

MICB_CONFIGURE_PRG = ./configure

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

