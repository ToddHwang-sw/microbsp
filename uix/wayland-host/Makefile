DIR=wayland
LOC=https://gitlab.freedesktop.org/wayland

BUILDDIRNAME=build

download:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(DIR) ] || ( cd $(BUILDDIR); \
			git clone --single-branch $(LOC)/$(DIR).git ; \
			cd $(DIR); \
			cat ../../../patch/patch | patch -p 1 )

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

