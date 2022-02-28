MODMAKE=$(KERNELMAKE) -C $(KERNDIR)/$(KERNELVER) CROSS_COMPILE=$(CROSS_COMP_PREFIX)

MODDIRS=\
	sfs

all:
	@for dir in $(MODDIRS); do $(MODMAKE) M=`pwd`/$$dir modules ; done

clean:
	@for dir in $(MODDIRS); do \
		\rm -rf $$dir/*.o $$dir/*.ko $$dir/.*.cmd $$dir/*.mod.c $$dir/modules.* $$dir/Module.* $$dir/.tmp_versions ; done

install:
	@for dir in $(MODDIRS); do \
		$(MODMAKE) M=`pwd`/$$dir INSTALL_MOD_PATH=$(INSTALLDIR) modules_install ; done
	@find $(INSTALLDIR)/lib/ -type f -name "*.ko" -print | \
		sh $(TOPDIR)/scripts/modules-alias.sh $(INSTALLDIR)/etc/modules.conf


