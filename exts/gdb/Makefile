DIR=gdb-8.2
LOC=ftp://ftp.gnu.org/pub/gnu/gdb

include $(TOPDIR)/scripts/env.mk

CROSS_USER_CFLAGS=
CROSS_USER_LFLAGS=

##
## --enable-shared && --diable-static should be elminated from option ... 
##
MICB_CONFIGURE_OPTS := $(filter-out --enable-shared,$(MICB_CONFIGURE_OPTS))
MICB_CONFIGURE_OPTS := $(filter-out --disable-static,$(MICB_CONFIGURE_OPTS))

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz ; $(MICB_PATCH) )
	cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS) 

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


