
EXTRA_CFLAGS 	= -g -Wall -D_DEBUG_ -I$(abs_srcdir)/inc
##EXTRA_CFLAGS  += -DMEMORY_TRACK
LIBS	+= -lpthread

AM_CFLAGS = $(EXTRA_CFLAGS) $(CFLAGS) $(all_includes)

lib_LTLIBRARIES = libsjson.la

libsjson_la_LDFLAGS= -shared
libsjson_la_SOURCES = \
	kits/malloc.c \
	kits/mem.c    \
	json/json.c

## test program 
bin_PROGRAMS = json_main

json_main_SOURCES = src/json_main.c
json_main_LDFLAGS = $(CROSS_COMP_FLAGS)
json_main_LDADD  = $(lib_LTLIBRARIES)

includelibsjsondir = $(includedir)/lsjson

includelibsjson_HEADERS = inc/common.h \
		   inc/json.h \
		   inc/malloc.h \
		   inc/mem.h
