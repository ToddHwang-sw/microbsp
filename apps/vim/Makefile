DIR=vim
LOC=https://github.com/vim

include $(TOPDIR)/scripts/env.mk

CROSS_USER_CFLAGS += -DMB_LEN_MAX=16 

MICB_DEPENDS = ncursesw

MICB_CONFIGURE_RUNENV+=\
		vim_cv_toupper_broken=yes \
		vim_cv_getcwd_broken=yes \
		vim_cv_tgetent=yes \
		vim_cv_terminfo=yes \
		vim_cv_tty_group=yes \
		vim_cv_tty_mode=yes \
		vim_cv_stat_ignores_slash=yes \
		vim_cv_memmove_handles_overlap=yes \
		vim_cv_bcopy_handles_overlap=yes \
		vim_cv_memcpy_handles_overlap=yes

MICB_CONFIGURE_PRG   = ./configure

MICB_CONFIGURE_OPTS += \
		--disable-arabic --disable-sysmouse --disable-nls --with-tlib=ncursesw

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR) ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git ; $(MICB_PATCH) )

prepare:
	@$(MICB_DUP_SOURCES)
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS) $(MICB_CONFIGURE_CMD) $(MICB_CONFIGURE_OPTS)

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make DESTDIR=$(destination) $@ )

