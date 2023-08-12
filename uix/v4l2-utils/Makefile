DIR=v4l-utils
LOC=-b stable-1.20 git://linuxtv.org

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_AUTOCONF_CMD = \
	aclocal ; autoreconf --install -v ; automake --add-missing --foreign

MICB_CONFIGURE_MAKEOPTS += \
	KERNEL_DIR=$(EXTINSTDIR) KERNEL_SRC_DIR=$(KERNELDIR)/$(KERNELVER) 

MICB_CONFIGURE_PRG = ./configure

download:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR); \
		git clone --single-branch $(LOC)/$(DIR).git ; \
		cd $(BUILDDIR)/$(DIR); \
		cat    ../../../patch/patch | patch -p 1; )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		aclocal ; \
		autoreconf --install -v; \
		automake --add-missing --foreign ; \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS) 

all install uninstall clean sync-with-kernel:
	[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


