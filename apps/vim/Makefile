DIR=vim
LOC=https://github.com/vim

include $(TOPDIR)/scripts/env.mk

CROSS_USER_CFLAGS += -DMB_LEN_MAX=16 

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || ( \
		[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR); \
		$(TOPDIR)/scripts/setupsrcs.sh $(BUILDDIR)/$(DIR) $(LOC)/$(DIR).git; \
		[ ! -f $(MICB_PATCH_FN) ] || ( \
			cd $(BUILDDIR)/$(DIR); cat ../../../$(MICB_PATCH_FN) | patch -p 1 ) \
	)

prepare:
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS)  \
		LIBS="-L$(destination)/lib -L$(destination)/lib64 -lncursesw -ltinfow -lrt -lpthread" \
		vim_cv_toupper_broken=yes \
		vim_cv_getcwd_broken=yes \
		vim_cv_tgetent=yes \
		vim_cv_terminfo=yes \
		vim_cv_tty_group=yes \
		vim_cv_tty_mode=yes \
		vim_cv_stat_ignores_slash=yes \
		vim_cv_memmove_handles_overlap=yes \
		vim_cv_bcopy_handles_overlap=yes \
		vim_cv_memcpy_handles_overlap=yes \
		./configure --prefix= --target=$(PLATFORM) --build=i686-linux --host=$(PLATFORM) \
			--disable-arabic --disable-sysmouse --disable-nls --with-tlib=ncursesw

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make DESTDIR=$(destination) $@ )

