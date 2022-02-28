/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2019  Red Hat, Inc.

  Logging API.

  This program can be distributed under the terms of the GNU LGPLv2.
  See the file COPYING.LIB
*/

#include "fuse_log.h"

#include <stdarg.h>
#include <stdio.h>

static void default_log_func(
		__attribute__(( unused )) enum fuse_log_level level,
#ifdef EXTRA_WORKAROUND
		const char *fn, const char *fmt, va_list ap)
#else
		const char *fmt, va_list ap)
#endif
{
	vfprintf(stderr, fmt, ap);
}

static fuse_log_func_t log_func = default_log_func;

void fuse_set_log_func(fuse_log_func_t func)
{
	if (!func)
		log_func = default_log_func;

	log_func = func;
}

#ifdef EXTRA_WORKAROUND
void __fuse_log(enum fuse_log_level level, const char *fn, const char *fmt, ...)
#else
void fuse_log(enum fuse_log_level level, const char *fmt, ...)
#endif
{
	va_list ap;

	va_start(ap, fmt);
#ifdef EXTRA_WORKAROUND
	log_func(level, fn, fmt, ap);
#else
	log_func(level, fmt, ap);
#endif
	va_end(ap);
}
