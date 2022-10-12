DIR=vim
LOC=https://github.com/vim
FILENM=vim.git

##
## No way with scripts/env.mk -- 
include $(TOPDIR)/arch/$(_ARCH_)/env.mk

CROSS_USER_CFLAGS += -DMB_LEN_MAX=16 

prepare:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)
	@[ -d $(BUILDDIR)/$(DIR)/.git ] || ( cd $(BUILDDIR); git clone $(LOC)/$(FILENM) )
	@cd $(BUILDDIR)/$(DIR); \
		$(COMP_CMDS)  \
		LIBS="-lncurses -ltinfo" \
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
			--disable-arabic --disable-sysmouse --disable-nls --with-tlib=ncurses

all install clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make $(MICB_CONFIGURE_MAKEOPTS) $@ )

