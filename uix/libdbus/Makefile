DIR=dbus
LOC=-b dbus-1.12 https://gitlab.freedesktop.org/dbus

include $(TOPDIR)/scripts/uixenv.mk

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_OPTS += \
	--disable-xml-docs \
	--disable-doxygen-docs \
	--disable-ducktype-docs \
	--disable-selinux \
	--enable-systemd=no

MICB_CONFIGURE_MAKEOPTS += V=1 

MICB_DEPENDS = intl iconv gio-unix-2.0 expat

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
	@$(MICB_CONFIGURE_AUTOCONF) 
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )



