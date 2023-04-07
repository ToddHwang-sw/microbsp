DIR=curl-7.81.0
LOC=https://curl.se/download

include $(TOPDIR)/scripts/extenv.mk

MICB_CONFIGURE_OPTS += \
	--enable-debug \
	--enable-http \
	--enable-ftp \
	--enable-rtsp \
	--enable-proxy \
	--enable-tftp \
	--enable-telnet \
	--enable-pop3 \
	--enable-imap \
	--enable-smb \
	--enable-smtp \
	--enable-gopher \
	--enable-mqtt \
	--enable-http-auth \
	--with-gnutls=$(EXTINSTDIR) \
	--with-openssl=$(INSTALLDIR) \
	--with-libssh2=$(INSTALLDIR) \
	--disable-openssl-auto-load-config 

MICB_CONFIGURE_RUNENV += \
	PKG_CONFIG_PATH=$(MICB_PKGCONFIG_PATH)

MICB_DEPENDS = gnutls openssl zlib

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -f $(MICBSRC)/$(DIR).tar.gz ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).tar.gz ; $(MICB_PATCH) )
	cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


