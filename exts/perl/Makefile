DIR=perl-5.24.1
LOC=https://www.cpan.org/src/5.0
FILENM=$(DIR).tar.gz
CROSS_PATCH_LOC=https://github.com/arsv/perl-cross/releases/download/1.1.3
CROSS_PATCH=perl-cross-1.1.3.tar.gz

prepare:
	@[ -d $(BUILDDIR) ] || mkdir -p $(BUILDDIR)
	@[ -f $(MICBSRC)/$(FILENM) ] || wget -P $(MICBSRC) $(LOC)/$(FILENM)
	@[ -f $(MICBSRC)/$(CROSS_PATCH) ] || wget -P $(MICBSRC) $(CROSS_PATCH_LOC)/$(CROSS_PATCH)
	@[ -f $(BUILDDIR)/$(DIR)/Makefile ] || \
	( tar zxvf $(MICBSRC)/$(FILENM) -C $(BUILDDIR) ; \
		cd $(BUILDDIR)/$(DIR); \
		tar --strip-components=1 -zxf ../../../$(MICBSRC)/$(CROSS_PATCH); \
		./configure \
			-Accflags="$(CROSS_COMP_FLAGS)" \
			-Dusethreads \
			--target=$(PLATFORM) \
			--target-tools-prefix=$(CROSS_COMP_PREFIX) \
			--prefix=/usr \
			--host-cc="gcc" \
			--host-cpp="g++" \
			--host-ranlib="ranlib" \
			--host-objdump="objdump" \
			--all-static )

all install uninstall clean:
	@[ ! -d $(BUILDDIR)/$(DIR) ] || ( \
		cd $(BUILDDIR)/$(DIR); make DESTDIR=$(destination)/ $@ )

