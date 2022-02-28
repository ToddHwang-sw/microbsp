DIR=X11

include $(TOPDIR)/scripts/x11uixenv.mk

setup_x11_basic:
	@[ -d $(X11INSTDIR)          ] || mkdir -p $(X11INSTDIR)
	@[ -d $(X11INSTDIR)/share    ] || mkdir -p $(X11INSTDIR)/share

prepare all install uninstall clean: setup_x11_basic


