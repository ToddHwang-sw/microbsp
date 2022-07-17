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
	--with-libiconv-prefix=$(EXTINSTDIR) \
	--without-libtextstyle 

##
## DESTDIR will screw all up. 
MICB_CONFIGURE_MAKEOPTS = V=1 prefix=$(destination) 

MICB_DEPENDS = intl zlib liblzma

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

