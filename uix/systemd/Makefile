DIR=systemd-stable
LOC=-b v250-stable https://github.com/systemd

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS +=\
        -D install-tests=false \
        -D tests=false \
		-D audit=false \
		-D seccomp=true \
		-D tpm=false \
		-D tpm2=false

MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(WAYLAND_LOCATION)/lib/$(HOSTSYSTEM)-gnu/pkgconfig

MICB_DEPENDS = iconv intl iconv libpcre glib-2.0 libffi zlib mount blkid pam liblzma liblz4 libip4tc libcap \
	       gnutls libssh2 ncurses libmnl libnftnl libzstd libseccomp libcurl

CROSS_USER_CFLAGS += \
	-DLINE_MAX=2048    \
	-DIOV_MAX=1024     \
	-DHOST_NAME_MAX=64

download:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )


