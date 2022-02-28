LOC=-b 2.02-coreos https://github.com/coreos
DIR=grub
FILENM=$(DIR).git

prepare:
	@[ -d source/$(FILENM) ] || ( \
		cd source; \
		git clone $(LOC)/$(FILENM) ; \
		cd $(DIR); \
		./autogen.sh )
	@[ -d build/$(DIR) ] || mkdir -p build/$(DIR)
	@cd build/$(DIR); \
		$(COMP_CMDS) \
		grub_cv_cc_no_red_zone=yes \
		../../source/$(DIR)/configure --prefix= --target=$(PLATFORM) --build=i686-linux --host=$(PLATFORM)  \
			--disable-nls -disable-rpath --with-platform=efi 

all clean:
	@[ ! -d build/$(DIR) ] || ( \
		make -C build/$(DIR) $@ )

install:
	@echo "$(DIR) will not be installed."

config:
	@echo "$(DIR) doesn't support config."

