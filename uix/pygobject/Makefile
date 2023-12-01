DIR=pygobject 
LOC=-b pygobject-3-34 https://github.com/GNOME

include $(TOPDIR)/scripts/uixenv.mk

MICB_MESON_OPTS += \
	-D tests=false  \
	-D pycairo=false

MICB_DEPENDS = gobject-introspection-1.0 python-$(PYTHON_SYSVER)-embed

download:
	@[ -d $(BUILDDIR)/$(DIR)  ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)   ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )
	@[ ! -d $(MICBSRC)/$(DIR) ] || $(MICB_MESON_CROSSBUILD_FILE)

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(MICB_MESON_CMD) $(MICB_MESON_OPTS)

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); $(NINJA_MAKE) $@ )

