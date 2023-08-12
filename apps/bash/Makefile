DIR=$(BASH)
LOC=https://ftp.gnu.org/gnu/bash

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_OPTS += \
	--disable-nls \
	--disable-rpath \
	--enable-job-control \
	--disable-help-builtin \
	--without-afs \
	--with-bash-malloc \
	--with-curses \
	--with-installed-readline \
	--without-libiconv-prefix \
	--with-included-gettext \
	--without-libintl-prefix 

MICB_DEPENDS = ncurses readline

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )
	@if [ "$@" = "install" ]; then  \
		([ -f $(destination)/bin/sh ] || (cd $(destination)/bin; ln -s bash sh)); fi


