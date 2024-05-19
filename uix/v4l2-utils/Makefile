DIR=v4l-utils
LOC=-b stable-1.20 git://linuxtv.org

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_AUTOCONF_CMD = \
	aclocal ; autoreconf --install -v ; automake --add-missing --foreign

MICB_CONFIGURE_MAKEOPTS += \
	KERNEL_DIR=$(EXTINSTDIR) KERNEL_SRC_DIR=$(KERNELDIR)/$(KERNELVER) 

MICB_CONFIGURE_PRG = ./configure

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		aclocal ; \
		autoreconf --install -v; \
		automake --add-missing --foreign ; \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS) 

all install uninstall clean sync-with-kernel:
	[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


