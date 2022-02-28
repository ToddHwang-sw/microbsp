
include $(TOPDIR)/scripts/env.mk

export CROSS_USER_CFLAGS += \
	-I$(EXTINSTDIR)/include \
	-I$(EXTINSTDIR)/usr/local/include \
	-I$(EXTINSTDIR)/usr/local/include/glib-2.0 \
	-I$(EXTINSTDIR)/usr/local/lib/glib-2.0/include 

export CROSS_USER_LFLAGS_EXTENSION = \
	-L$(EXTINSTDIR)/lib \
	-L$(EXTINSTDIR)/lib64 \
	-L$(EXTINSTDIR)/usr/local/lib 

export LIBS_EXTENSION = \
	$(call build_libs,$(EXTINSTDIR)/lib)   \
      	$(call build_libs,$(EXTINSTDIR)/lib64) \
      	$(call build_libs,$(EXTINSTDIR)/usr/local/lib)

export CROSS_USER_LFLAGS += $(CROSS_USER_LFLAGS_EXTENSION)

export MICB_PKGCONFIG_PATH := $(MICB_PKGCONFIG_PATH):$(EXTINSTDIR)/lib/pkgconfig:$(EXTINSTDIR)/share/pkgconfig:$(EXTINSTDIR)/usr/local/lib/pkgconfig:$(EXTINSTDIR)/usr/local/share/pkgconfig

##
## GNU S/W bootstrap process takes very long time, so we use this file to prevent multiple bootstrapping by uncareful configuration challenges..
export BSDONE_FILE = ./__bootstrap.done

