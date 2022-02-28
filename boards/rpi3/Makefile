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
	@cd $(isodir)/boot; \
		tar zcvf ../../$(BDTARGZ) * > /dev/null

uninstall:
	@[ ! -f $(BDTARGZ) ] || \rm -rf $(BDTARGZ)
	
clean:
	@make -C firmware clean 
	@make -C boot isodir=$(isodir) clean

distclean:
	@make -C firmware distclean 
	@make -C boot isodir=$(isodir) clean

