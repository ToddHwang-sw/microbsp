
BUILDDIRNAME=__build__

CROSS_LFLAG_EXTRA = \
	$(LIBS_BASIC)    \
	$(LIBS_GLIB)     \
	$(LIBS_FFMPEG)   \
	$(LIBS_X11)      \
	$(LIBS_UIBASIC)  \
	-lglapi -ldbus-1 \
	-lpcre2-posix -lpcre2-8  -lpcre2-16 -lpcre2-32 \
	-lsndfile        \
	-lswresample     \
	-lva             \
	-lswscale        \
	-lpostproc       \
	-lasound

CROSS_USER_CFLAGS += \
	-I$(UIXINSTDIR)/include/alsa \
	-I$(UIXINSTDIR)/usr/local/lib/gstreamer-1.0/include

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR) ;       \
		git clone --single-branch $(LOC)/$(DIR).git ; \
		[ ! -f ../../patch/patch ] || ( \
			cd $(DIR); \
			cat ../../../patch/patch | patch -p 1 ) \
		)
	@[ -d $(BUILDDIR)/$(DIR)/$(BUILDDIRNAME) ] || \
		mkdir -p $(BUILDDIR)/$(DIR)/$(BUILDDIRNAME)
	@$(MICB_MESON_CROSSBUILD_FILE)
	cd $(BUILDDIR)/$(DIR); 								\
		$(MICB_MESON_RUNENV)                                                    \
		meson . $(BUILDDIRNAME) --cross-file ../$(MICB_MESON_CROSSBUILD_FN) 	\
			$(GSTREAMER_OPTS) $(MICB_MESON_OPTS)

all:
	[ ! -d $(BUILDDIR)/$(DIR)/$(BUILDDIRNAME) ] || ( \
		cd $(BUILDDIR)/$(DIR)/$(BUILDDIRNAME); DESTDIR=$(destination) $(GSTREAMER_NINJA_ENV) ninja -v )

install clean:
	[ ! -d $(BUILDDIR)/$(DIR)/$(BUILDDIRNAME) ] || ( \
		cd $(BUILDDIR)/$(DIR)/$(BUILDDIRNAME); DESTDIR=$(destination) $(GSTREAMER_NINJA_ENV) ninja -v $@ )

