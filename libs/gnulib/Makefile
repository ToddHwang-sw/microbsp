DIR=gnulib
LOC=-b stable-202207 https://github.com/coreutils

include $(TOPDIR)/scripts/env.mk

download:
	@[ -d $(BUILDDIR)/$(DIR) ] || mkdir -p $(BUILDDIR)/$(DIR)
	@[ -d $(MICBSRC)/$(DIR)  ] || ( $(UNCOMPRESS) $(LOC)/$(DIR).git; $(MICB_PATCH) )

prepare:
		
all:

install uninstall clean:

