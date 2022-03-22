DIR=ncurses-5.6
LOC=ftp://ftp.gnu.org/gnu/ncurses

include $(TOPDIR)/scripts/env.mk

HOSTOPT=$(PLATFORM)
ifeq ($(_CORE_),aarch64)
	MICB_CONFIGURE_PLATFORM=arm-linux
endif

MICB_CONFIGURE_OPTS +=\
	--without-sysmouse \
	--without-cxx-binding \
	--without-ada \
	--without-termcap \
	--with-termlib \
	--with-terminfo \
	--with-shared=yes \
	--with-normal=no

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( \
		$(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


