DIR=wayland
LOC=https://gitlab.freedesktop.org/wayland

include $(TOPDIR)/scripts/uixenv.mk

BUILDDIRNAME=build

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || ( \
		[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR); \
		$(TOPDIR)/scripts/setupsrcs.sh $(BUILDDIR)/$(DIR) $(LOC)/$(DIR).git; \
		[ ! -f $(MICB_PATCH_FN) ] || ( \
			cd $(BUILDDIR)/$(DIR); cat ../../../$(MICB_PATCH_FN) | patch -p 1 ) \
	)

prepare:
	@cd $(BUILDDIR); \
			mkdir -p $(DIR)/$(BUILDDIRNAME); \
			cd $(DIR); \
			meson . $(BUILDDIRNAME) \
				-D documentation=false \
				-D tests=false \
				-D libraries=false \
				-D scanner=true \
				-D prefix=$(TOPDIR)/uix/wayland-host/build/host

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); \
		ninja -v -C $(BUILDDIRNAME)/ $@ )

