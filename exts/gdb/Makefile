DIR=gdb-11.2
LOC=https://ftp.gnu.org/pub/gnu/gdb

include $(TOPDIR)/scripts/extenv.mk

CROSS_USER_CFLAGS+=\
		-DMB_LEN_MAX=16

MICB_DEPENDS = mpfr mpc gmp

##
## --enable-shared && --diable-static should be elminated from option ... 
##
MICB_CONFIGURE_OPTS := $(filter-out --enable-shared,$(MICB_CONFIGURE_OPTS))
MICB_CONFIGURE_OPTS := $(filter-out --disable-static,$(MICB_CONFIGURE_OPTS))

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz ; $(MICB_PATCH) )

prepare:
	cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS) 

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


