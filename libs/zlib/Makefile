DIR=zlib-1.2.12
LOC=https://www.zlib.net

include $(TOPDIR)/scripts/env.mk

MICB_CONFIGURE_CMD=../../../$(MICBSRC)/$(DIR)/configure 
MICB_CONFIGURE_OPTS = --prefix= --enable-shared

CROSS_USER_CFLAGS += -fPIC 

prepare: 
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz; $(MICB_PATCH) )
	cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) LDSHARED="$(CC) -shared" $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

