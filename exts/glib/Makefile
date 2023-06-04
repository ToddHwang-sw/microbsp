DIR=glib
LOC=-b glib-2-70 https://gitlab.gnome.org/GNOME

include $(TOPDIR)/scripts/extenv.mk

CROSS_USER_CFLAGS += \
	-D_POSIX_HOST_NAME_MAX=255

## Ubuntu 21.xx LTS
CROSS_USER_LFLAGS += -lresolv

MICB_MESON_OPTS+=\
	-D gtk_doc=false \
	-D glib_debug=disabled \
	-D libelf=disabled \
	-D selinux=disabled \
	-D tests=false \
	-D man=false \
	-D installed_tests=false 

MICB_DEPENDS = intl iconv libpcre libffi zlib mount blkid ncurses

prepare:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )

