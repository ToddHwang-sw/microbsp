DIR=xcbproto
LOC=-b xcb-proto-1.16.0 https://gitlab.freedesktop.org/xorg/proto

include $(TOPDIR)/scripts/x11uixenv.mk

MICB_CONFIGURE_PRG = ./configure

MICB_CONFIGURE_RUNENV += \
	PYTHON=/usr/bin/python3

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


