DIR=gettext-0.21
LOC=https://ftp.gnu.org/pub/gnu/gettext

include $(TOPDIR)/scripts/extenv.mk

## gettext does not allow external XML library 
CROSS_USER_CFLAGS := $(filter-out -I$(INSTALLDIR)/include/libxml2,$(CROSS_USER_CFLAGS))

MICB_CONFIGURE_OPTS += \
	--enable-largefile \
	--enable-threads=posix \
	--enable-curses \
	--with-bzip2  \
	--with-libxml2-prefix=$(INSTALLDIR) \
	--with-libncurses-prefix=$(INSTALLDIR) \
	--with-libiconv-prefix=$(EXTINSTDIR)

MICB_DEPENDS = intl zlib liblzma ncurses

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make V=1 $@ )

install uninstall:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )
