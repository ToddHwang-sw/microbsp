/**
 * @file     xprint.h
 * @brief    SCEW print functions
 * @author   Aleix Conchillo Flaque <aleix@member.fsf.org>
 * @date     Sun Mar 30, 2003 13:30
 *
 * @if copyright
 *
 * Copyright (C) 2003, 2004, 2005, 2006, 2007 Aleix Conchillo Flaque
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


#ifndef XPRINT_H_0303301330
#define XPRINT_H_0303301330

#include "tree.h"

#include <stdio.h>

/* Prints an XML tree into a file. */
extern void tree_print (scew_tree const *tree, FILE *out);

#endif /* XPRINT_H_ALEIX0303301330 */
