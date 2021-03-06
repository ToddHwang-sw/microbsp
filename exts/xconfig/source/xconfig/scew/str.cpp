/**
 * @file     str.c
 * @brief    str.h implementation
 * @author   Aleix Conchillo Flaque <aleix@member.fsf.org>
 * @date     Mon Nov 25, 2002 23:32
 *
 * @if copyright
 *
 * Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007 Aleix Conchillo Flaque
 *
 * SCEW is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * SCEW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * @endif
 */

#include "str.h"

#include "xerror.h"
#include "str.h"

#include <assert.h>

#ifdef XCONFIG_PATCH
	#include "malloc.h"
	#include "xmlmem.h"
#endif /* XCONFIG_PATCH */

XML_Char*
scew_strdup (XML_Char const *src)
{
	assert (src != NULL);

	unsigned int len = scew_strlen (src);
#ifdef XCONFIG_PATCH
	XML_Char *out = (XML_Char *)os_calloc (len + 1, sizeof (XML_Char));
#else
	XML_Char *out = (XML_Char *)calloc (len + 1, sizeof (XML_Char));
#endif

	return (XML_Char*) scew_memcpy (out, (XML_Char*) src, len);
}

void
scew_strtrim (XML_Char *src)
{
	assert (src != NULL);

	unsigned int end = scew_strlen (src);

	/* strip trailing whitespace */
	while (end > 0 && scew_isspace (src[end-1]))
	{
		src[--end] = '\0';
	}

	/* strip leading whitespace */
	unsigned int start = scew_strspn (src, _XT (" \n\r\t\v") );
	unsigned int total = end - start;
	scew_memmove (src, &src[start], total);
	src[total] = _XT ('\0');
}
