DIR=libxcb
LOC=-b libxcb-1.16 https://gitlab.freedesktop.org/xorg/lib

include $(TOPDIR)/scripts/x11uixenv.mk

MICB_CONFIGURE_RUNENV += \
	PYTHON=/usr/bin/python3 

MICB_CONFIGURE_MAKEOPTS += \
	XCBPROTO_XCBINCLUDEDIR=$(X11INSTDIR)/share/xcb \
	XCBPROTO_XCBPYTHONDIR=$(X11INSTDIR)/lib/python$(PYTHON_SYSVER)/site-packages

MICB_DEPENDS = xproto xau

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	@$(MICB_CONFIGURE_AUTOCONF)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
                cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )


