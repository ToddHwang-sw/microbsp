BOOTARGS="console=ttyPS0,115200 earlyprintk root=/dev/ram0 ro rootfstype=squashfs rootwait"
ZIPEXIST=`command -v 7z | wc -l`
BDIMAGE=boot.7z
BDTARGZ=boot.tgz

prepare:
	@[ -d $(isodir) ]            || mkdir -p $(isodir)
	@[ -d $(isodir)/boot ]       || mkdir -p $(isodir)/boot
	@make -C firmware prepare 
	@make -C boot     isodir=$(isodir) prepare all

all:
	@echo "Image nothing to be compiled"

install:
	@make -C firmware isodir=$(isodir) install
	@make -C boot     isodir=$(isodir) install
	@\rm -rf $(isodir)/boot/*.old
	@echo ""
	@echo "Compressing...."
	@echo ""
	@[ "$(ZIPEXIST)" = "" ] || ( \
		[ ! -f $(BDIMAGE) ] || \rm -rf $(BDIMAGE) ; \
		cd $(isodir)/boot; \
			7z a ../../$(BDIMAGE) * \
	)
	@cd $(isodir)/boot; \
		tar zcvf ../../$(BDTARGZ) * > /dev/null

clean:
	@make -C firmware clean 
	@make -C boot isodir=$(isodir) clean

