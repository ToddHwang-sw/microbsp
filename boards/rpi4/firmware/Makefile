LOC=https://github.com/raspberrypi/firmware/raw/stable/boot/
FWS=bootcode.bin start4.elf start4x.elf fixup4.dat fixup4x.dat
FILES=$(FWS) config.txt cmdline.txt

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

