LOC=https://mirrors.edge.kernel.org/pub/linux/utils/boot/syslinux/Testing/6.04
DIR=syslinux-6.04-pre1
FILENM=$(DIR).tar.gz

prepare:
	@[ -f source/$(FILENM) ] || wget -P source $(LOC)/$(FILENM)
	@[ -d build/$(DIR) ] || tar zxvf source/$(FILENM) -C build 

all clean:
	@[ -s build/syslinux ] || ( \
		make -C build/$(DIR) $@ ; \
		cd build; \
		ln -s $(DIR) syslinux )

install:
	@echo "$(DIR) will not be installed."

config:
	@echo "$(DIR) doesn't support config."

