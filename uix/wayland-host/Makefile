DIR=wayland
LOC=https://gitlab.freedesktop.org/wayland

include $(TOPDIR)/scripts/uixenv.mk

BUILDDIRNAME=build

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@$(MICB_DUP_SOURCES)

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

