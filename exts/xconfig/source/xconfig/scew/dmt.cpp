#include "element.h"
#include "tree.h"
#include "list.h"
#include "str.h"
#include "dmt.h"

#ifdef XCONFIG_PATCH
	#include "malloc.h"
	#include "xmlmem.h"
#endif

static scew_list *str2list(XML_Char *str, const XML_Char *delm)
{
	XML_Char *token, *tmp_str;
	scew_list *list = NULL;

	/* Avoid warning: tmp_str is unused */
	(void) tmp_str;

	for (;; str = NULL) {
		token = scew_strtok(str, delm, &tmp_str);
		if (token == NULL)
			break;
		if (list)
			scew_list_append(list, token);
		else
			list = scew_list_create(token);
	}

	return list;
}

static scew_list *__node_list_by_uri(scew_element *e, scew_list *str_list, scew_list *res_list)
{
	scew_list *list, *next_str, *item;
	scew_element *child, *parent;
	XML_Char const *node_name;
	XML_Char *str;

	if (e == NULL)
		return res_list;

	list = scew_element_children(e);
	str = (XML_Char *)scew_list_data(str_list);

	next_str = scew_list_next(str_list);
	if (!scew_strcmp(str, _XT(".."))) {
		parent = scew_element_parent(e);
		if (parent) {
			if (next_str)
				res_list = __node_list_by_uri(parent, next_str, res_list);
			else if (scew_list_find(res_list, parent) == NULL) {
				item = scew_list_append(res_list, parent);
				if (res_list == NULL)
					res_list = item;
			}
		}
		return res_list;
	}

	while (list) {
		child = (scew_element *)scew_list_data(list);

		node_name = scew_element_name(child);
		if (node_name) {
			if (!scew_strcmp(str, _XT("*")) || !scew_strcmp(node_name, str)) {
				if (next_str)
					res_list = __node_list_by_uri(child, next_str, res_list);
				else if (scew_list_find(res_list, child) == NULL) {
					item = scew_list_append(res_list, child);
					if (res_list == NULL)
						res_list = item;
				}
			}
		}
		list = scew_list_next(list);
	}

	return res_list;
}

scew_list *node_list_by_uri(scew_element *e, const XML_Char *uri)
{
	XML_Char *str;
	scew_list *list, *ret_list;

	str = scew_strdup(uri);
	if (str == NULL)
		return NULL;

	list = str2list(str, _XT("/"));
	if (list == NULL) {
		free(str);
		return NULL;
	}

	ret_list = __node_list_by_uri(e, list, NULL);

#ifdef XCONFIG_PATCH
	os_free(str);
#else
	free(str);
#endif
	scew_list_free(list);

	return ret_list;
}

