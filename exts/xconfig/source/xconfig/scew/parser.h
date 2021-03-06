/**
 * @file     parser.h
 * @brief    SCEW parser type declaration
 * @author   Aleix Conchillo Flaque <aleix@member.fsf.org>
 * @date     Mon Nov 25, 2002 00:57
 * @ingroup  SCEWParser, SCEWParserAlloc, SCEWParserLoad, SCEWParserAcc
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
 *
 */

/**
 * @defgroup SCEWParser Parser
 *
 * These are the parser functions that allow to read an XML tree from a
 * file or a memory buffer.
 */

#ifndef PARSER_H_0211250057
#define PARSER_H_0211250057

#include "tree.h"

#include <expat.h>

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * This is the type declaration of the SCEW parser.
 *
 * @ingroup SCEWParser
 */
typedef struct scew_parser scew_parser;

/**
 * Callback function type.
 *
 * @return true if callback call had no errors, false otherwise.
 *
 * @ingroup SCEWParserLoad
 */
typedef bool (*scew_parser_callback) (scew_parser* parser);


/**
 * @defgroup SCEWParserAlloc Allocation
 * Allocate and free a parser.
 * @ingroup SCEWParser
 */

/**
 * Creates a new parser. The parser is needed to load XML documents.
 *
 * @ingroup SCEWParserAlloc
 */
extern scew_parser* scew_parser_create (void);

/**
 * Frees a parser memory structure. This function will not free the
 * #scew_tree generated by the parser, so it is important that you
 * keep a pointer to it and remember to free it.
 *
 * @see scew_tree_free
 *
 * @ingroup SCEWParserAlloc
 */
extern void scew_parser_free (scew_parser *parser);


/**
 * @defgroup SCEWParserLoad Load
 * Load XML documents from different sources.
 * @ingroup SCEWParser
 */

/**
 * Loads an XML tree from the specified file name using the given
 * parser.
 *
 * @param parser the SCEW parser.
 * @param file_name the file to load the XML from.
 *
 * @see scew_parser_create
 *
 * @return true if file was successfully loaded, false otherwise.
 *
 * @ingroup SCEWParserLoad
 */
extern bool scew_parser_load_file (scew_parser *parser, char const *file_name);

/**
 * Loads an XML tree from the specified file pointer using the
 * given parser.
 *
 * @param parser the SCEW parser.
 * @param in the file pointer to load the XML from.
 *
 * @see scew_parser_create
 *
 * @return true if file was successfully loaded, false otherwise.
 *
 * @ingroup SCEWParserLoad
 */
extern bool scew_parser_load_file_fp (scew_parser *parser, FILE *in);

/**
 * Loads an XML tree from the specified memory buffer of the specified
 * size using the given parser.
 *
 * @param parser the SCEW parser.
 * @param buffer memory buffer to load XML from.
 * @param size size in bytes of the memory buffer.
 *
 * @see scew_parser_create
 *
 * @return true if buffer was successfully loaded, false otherwise.
 *
 * @ingroup SCEWParserLoad
 */
extern bool scew_parser_load_buffer (scew_parser *parser,
                                     char const *buffer,
                                     unsigned int size);

/**
 * Loads an XML tree from the specified stream buffer. Will call the
 * callback (set using #scew_parser_set_stream_callback) at the end of
 * each message.
 *
 * @param parser the SCEW parser.
 * @param buffer memory buffer to load XML from.
 * @param size size in bytes of the memory buffer.
 *
 * @see scew_parser_create
 * @see scew_parser_set_stream_callback
 *
 * @return true if buffer was successfully loaded, false otherwise.
 *
 * @ingroup SCEWParserLoad
 */
extern bool scew_parser_load_stream (scew_parser *parser,
                                     char const *buffer,
                                     unsigned int size);

/**
 * Sets the callback for use when reading streams.
 *
 * @param parser the SCEW parser
 * @param cb the callback function
 *
 * @ingroup SCEWParserLoad
 */
extern void scew_parser_set_stream_callback (scew_parser *parser,
                                             scew_parser_callback cb);

/**
 * Tells the parser how to treat white spaces. The default is to
 * ignore heading and trailing white spaces.
 *
 * There is a new section in XML specification which talks about how
 * to handle white spaces in XML. One can set an optional attribtue to
 * an element, this attribute is called 'xml:space', and it can be set
 * to 'default' or 'preserve', and it inherits its value from parent
 * elements. 'preserve' means to leave white spaces as their are
 * found, and 'default' means that white spaces are handled by the XML
 * processor (Expat in our case) the way it wants to.
 *
 * This function gives the possibility to change the XML processor
 * behaviour.
 *
 * @param parser the parser to set the option to.
 * @param ignore whether the parser should ignore white spaces, false
 * otherwise.
 *
 * @ingroup SCEWParserLoad
 */
extern void scew_parser_ignore_whitespaces (scew_parser *parser, bool ignore);


/**
 * @defgroup SCEWParserAcc Accessors
 * Obtain information from parser.
 * @ingroup SCEWParser
 */

/**
 * Returns the XML tree read by the parser. Remember that
 * #scew_parser_free does not free the #scew_tree read.
 *
 * @see tree.h
 *
 * @ingroup SCEWParserAcc
 */
extern scew_tree* scew_parser_tree (scew_parser const *parser);

/**
 * Returns the internal Expat parser. Probably some low-level Expat
 * functions need to be called. This function gives you access to the
 * Expat parser so you will be able to call those functions. If you
 * modify the Expat parser event handling routines, SCEW will not be
 * able to load the XML tree.
 *
 * @ingroup SCEWParserAcc
 */
extern XML_Parser scew_parser_expat (scew_parser *parser);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PARSER_H_0211250057 */
