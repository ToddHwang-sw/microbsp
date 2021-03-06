/**
 * @file     parser.c
 * @brief    parser.h implementation
 * @author   Aleix Conchillo Flaque <aleix@member.fsf.org>
 * @date     Mon Nov 25, 2002 00:58
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

#include "parser.h"

#include "xparser.h"
#include "xerror.h"
#include "xhandler.h"

#include "tree.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef XCONFIG_PATCH
#include "malloc.h"
#include "xmlmem.h"
const static XML_Memory_Handling_Suite xmlmem =
{
	.malloc_fcn  = __os_malloc,
	.realloc_fcn = __os_realloc,
	.free_fcn    = __os_free
};
#endif /* XCONFIG_PATCH */


// Private

static bool init_expat_parser_ (scew_parser *parser);


// Public

scew_parser*
scew_parser_create (void)
{
	scew_parser *parser;

#ifdef XCONFIG_PATCH
	parser = (scew_parser *)os_calloc (1, sizeof (scew_parser));
#else
	parser = (scew_parser *)calloc (1, sizeof (scew_parser));
#endif
	if (parser == NULL)
	{
		set_last_error (scew_error_no_memory);
		return NULL;
	}

	if (!init_expat_parser_ (parser))
	{
		scew_parser_free (parser);
		return NULL;
	}

	/* ignore white spaces by default */
	parser->ignore_whitespaces = true;

	/* no callback by default */
	parser->stream_callback = NULL;

	return parser;
}

void
scew_parser_free (scew_parser* parser)
{
	assert (parser != NULL);

	if (parser->parser)
	{
		XML_ParserFree (parser->parser);
	}
#ifdef XCONFIG_PATCH
	os_free (parser);
#else
	free (parser);
#endif
}

bool
scew_parser_load_file (scew_parser *parser, char const *file_name)
{
	assert (parser != NULL);
	assert (file_name != NULL);

	FILE *in = fopen (file_name, "rb");
	if (in == NULL)
	{
		set_last_error (scew_error_io);
		return 0;
	}

	bool result = scew_parser_load_file_fp (parser, in);
	fclose (in);

	return result;
}

bool
scew_parser_load_file_fp (scew_parser *parser, FILE *in)
{
	assert (parser != NULL);
	assert (in != NULL);

	bool done = false;
	while (!done)
	{
		enum { MAX_BUFFER = 5000 };
		char buffer[MAX_BUFFER];

		int len = fread (buffer, 1, MAX_BUFFER, in);
		if (ferror (in))
		{
			set_last_error (scew_error_io);
			return false;
		}

		done = feof (in);
		if (!XML_Parse (parser->parser, buffer, len, done))
		{
			set_last_error (scew_error_expat);
			return false;
		}
	}

	return true;
}

bool
scew_parser_load_buffer (scew_parser *parser, char const *buffer,
                         unsigned int size)
{
	assert (parser != NULL);
	assert (buffer != NULL);
	assert (size > 0);

	if (!XML_Parse (parser->parser, buffer, size, 1))
	{
		set_last_error (scew_error_expat);
		return false;
	}

	return true;
}

bool
scew_parser_load_stream (scew_parser *parser, char const *buffer,
                         unsigned int size)
{
	assert (parser != NULL);
	assert (buffer != NULL);
	assert (size > 0);

	unsigned int start = 0;
	unsigned int end = 0;

	/**
	 * Loop through the buffer.
	 * if we encounter a '>', send the chunk to Expat.
	 * if we hit the end of the buffer, send whatever remains to Expat.
	 * if the we have a full element (stack is empty) we call the callback.
	 */
	while ((start < size) && (end <= size))
	{
		if ((end == size) || (buffer[end] == '>'))
		{
			unsigned int length = end - start;
			if (end < size)
			{
				length = length + 1;
			}

			if (!XML_Parse (parser->parser, &buffer[start], length, 0))
			{
				set_last_error (scew_error_expat);
				return false;
			}

			if ((parser->tree != NULL) && (parser->current == NULL)
			    && (parser->stack == NULL) && parser->stream_callback)
			{
				/* tell Expat we're done */
				XML_Parse (parser->parser, "", 0, 1);

				/* call the callback */
				if (!parser->stream_callback (parser))
				{
					set_last_error (scew_error_callback);
					return false;
				}

				XML_ParserFree (parser->parser);
				scew_tree_free (parser->tree);
				parser->tree = NULL;
				init_expat_parser_ (parser);
			}
			start = end + 1;
		}
		++end;
	}

	return true;
}

void
scew_parser_set_stream_callback (scew_parser *parser, scew_parser_callback cb)
{
	assert (parser != NULL);

	parser->stream_callback = cb;
}

scew_tree*
scew_parser_tree (scew_parser const *parser)
{
	assert (parser != NULL);

	return parser->tree;
}

XML_Parser
scew_parser_expat (scew_parser *parser)
{
	assert (parser != NULL);

	return parser->parser;
}

void
scew_parser_ignore_whitespaces (scew_parser *parser, bool ignore)
{
	assert (parser != NULL);

	parser->ignore_whitespaces = ignore;
}


// Private

bool
init_expat_parser_ (scew_parser *parser)
{
	assert (parser != NULL);

#ifdef XCONFIG_PATCH
	parser->parser = XML_ParserCreate_MM (NULL,&xmlmem,NULL);
#else
	parser->parser = XML_ParserCreate (NULL);
#endif
	if (parser->parser == NULL)
	{
		set_last_error (scew_error_no_memory);
		return false;
	}

	/* initialize Expat handlers */
	XML_SetXmlDeclHandler (parser->parser, xmldecl_handler);
	XML_SetElementHandler (parser->parser, start_handler, end_handler);
	XML_SetCharacterDataHandler (parser->parser, char_handler);
	XML_SetUserData (parser->parser, parser);

	return true;
}
