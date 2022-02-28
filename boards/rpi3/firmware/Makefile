LOC=https://github.com/raspberrypi/firmware/raw/stable/boot/
FWS=bootcode.bin start.elf start_x.elf fixup.dat fixup_x.dat
FILES=$(FWS) config.txt cmdline.txt conf.sys

prepare:
	@for f in $(FWS); do \
		[ -f $$f ] || ( \
			wget $(LOC)/$$f; \
			[ "$$f" != "bootcode.bin" ] || sed -i -e "s/BOOT_UART=0/BOOT_UART=1/" $$f; \
		); done

all: 

clean:

distclean:
	@for f in $(FWS);  do \rm -rf $$f; done
	
install: 
	@for f in $(FILES);  do cp -rf $$f $(isodir)/boot; done

