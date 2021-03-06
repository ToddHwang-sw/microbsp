/**
 * @file     list.c
 * @brief    list.h implementation
 * @author   Aleix Conchillo Flaque <aleix@member.fsf.org>
 * @date     Thu Jul 12, 2007 20:09
 *
 * @if copyright
 *
 * Copyright (C) 2007 Aleix Conchillo Flaque
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
 **/

#include "list.h"

#include <assert.h>
#include <stdlib.h>

#ifdef XCONFIG_PATCH
	#include "malloc.h"
	#include "xmlmem.h"
#endif


/* Private */

struct scew_list
{
	void *data;
	scew_list *prev;
	scew_list *next;
};


// Public


// Allocation

scew_list*
scew_list_create (void *data)
{
	assert (data != NULL);

#ifdef XCONFIG_PATCH
	scew_list *list = (scew_list *)os_calloc (1, sizeof (scew_list));
#else
	scew_list *list = (scew_list *)calloc (1, sizeof (scew_list));
#endif

	if (list != NULL)
	{
		list->data = data;
	}

	return list;
}

void
scew_list_free (scew_list *list)
{
	while (list != NULL)
	{
		scew_list *tmp = list;
		list = list->next;
#ifdef XCONFIG_PATCH
		os_free (tmp);
#else
		free (tmp);
#endif
	}
}


// Accessors

void*
scew_list_data (scew_list *list)
{
	assert (list != NULL);

	return list->data;
}

unsigned int
scew_list_size (scew_list *list)
{
	unsigned int total = 0;

	while (list != NULL)
	{
		++total;
		list = list->next;
	}

	return total;
}


// Modifiers

scew_list*
scew_list_append (scew_list *list, void *data)
{
	assert (data != NULL);

	scew_list *item = scew_list_create (data);

	if (item != NULL)
	{
		if (list != NULL)
		{
			scew_list *last = scew_list_last (list);
			last->next = item;
			item->prev = last;
		}
	}

	return item;
}

scew_list*
scew_list_prepend (scew_list *list, void *data)
{
	assert (data != NULL);

	scew_list *item = scew_list_create (data);

	if (item != NULL)
	{
		if (list != NULL)
		{
			scew_list *first = scew_list_first (list);
			first->prev = item;
			item->next = first;
		}
	}

	return item;
}

scew_list*
scew_list_delete (scew_list *list, void *data)
{
	assert (list != NULL);
	assert (data != NULL);

	scew_list *tmp = list;

	while (tmp != NULL)
	{
		if (tmp->data != data)
		{
			tmp = tmp->next;
		}
		else
		{
			if (tmp->prev)
			{
				tmp->prev->next = tmp->next;
			}
			if (tmp->next)
			{
				tmp->next->prev = tmp->prev;
			}

			if (list == tmp)
			{
				list = list->next;
			}
			break;
		}
	}

	return list;
}

scew_list*
scew_list_delete_item (scew_list *list, scew_list *item)
{
	assert (list != NULL);
	assert (item != NULL);

	if (item != NULL)
	{
		if (item->prev != NULL)
		{
			item->prev->next = item->next;
		}
		if (item->next != NULL)
		{
			item->next->prev = item->prev;
		}

		if (item == list)
		{
			list = list->next;
		}

#ifdef XCONFIG_PATCH
		os_free (item);
#else
		free (item);
#endif
	}

	return list;
}


// Traverse

scew_list*
scew_list_first (scew_list *list)
{
	if (list != NULL)
	{
		while (list->prev != NULL)
		{
			list = list->prev;
		}
	}
	return list;
}

scew_list*
scew_list_last (scew_list *list)
{
	if (list != NULL)
	{
		while (list->next != NULL)
		{
			list = list->next;
		}
	}
	return list;
}

scew_list*
scew_list_next (scew_list *list)
{
	return (list != NULL) ? list->next : NULL;
}

scew_list*
scew_list_previous (scew_list *list)
{
	return (list != NULL) ? list->prev : NULL;
}

void
scew_list_foreach (scew_list *list, scew_list_function func, void *user_data)
{
	while (list != NULL)
	{
		func (list, user_data);
		list = list->next;
	}
}


// Search

scew_list*
scew_list_find (scew_list *list, void *data)
{
	while (list != NULL)
	{
		if (list->data == data)
		{
			break;
		}
		list = list->next;
	}

	return list;
}

scew_list*
scew_list_find_custom (scew_list *list,
                       void const *data,
                       scew_cmp_function func)
{
	while (list != NULL)
	{
		if (func (list->data, data))
		{
			break;
		}
		list = list->next;
	}

	return list;
}

scew_list*
scew_list_by_index (scew_list *list, unsigned int idx)
{
	unsigned int count = 0;

	while ((list != NULL) && (count < idx))
	{
		list = list->next;
		++count;
	}

	return list;
}
