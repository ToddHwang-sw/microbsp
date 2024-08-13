DIR=ncurses-6.3
LOC=ftp://ftp.gnu.org/gnu/ncurses

include $(TOPDIR)/scripts/env.mk

ifeq ($(_CORE_),aarch64)
	MICB_CONFIGURE_PLATFORM=arm-linux
endif

## TODO: 
## 
##	--enable-widec  ???
##

MICB_CONFIGURE_OPTS +=\
	--enable-largefile \
	--enable-widec \
	--enable-pc-files \
	--disable-stripping \
	--with-pkg-config-libdir=/lib/pkgconfig \
	--without-develop \
	--without-sysmouse \
	--without-cxx-binding \
	--without-ada \
	--without-termcap \
	--without-debug \
	--without-tests \
	--with-termlib \
	--with-terminfo \
	--with-shared=yes

## tinfo error ...
CROSS_USER_LFLAGS += -L$(INSTALLDIR)/lib

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )

prepare: 
	cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


