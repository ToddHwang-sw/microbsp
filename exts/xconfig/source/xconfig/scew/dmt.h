#ifndef __OMA_DMT_H__
#define __OMA_DMT_H__

#include "element.h"
#include "list.h"
#include "tree.h"
#include "parser.h"

#define XML_ROOT_NAME "config" // This means top tag name shuold be <XML_ROOT_STR>
#define XML_ROOT_NAME_LEN 4

extern scew_list *node_list_by_uri(scew_element *e, const XML_Char *uri);

/* --------------- User Functions ---------------- */

extern int config_xml_init(XML_Char *xml_file, scew_parser **g_parser, scew_tree **g_tree);
extern void config_xml_exit(scew_parser *g_parser, scew_tree *g_tree);
extern scew_list *config_xml_node_list_by_uri(scew_tree *g_tree, const XML_Char *uri);
extern int config_merge_to_xml(char* old_cfg, char* new_cfg);

/* Write oma tree to a file. */
int config_write_tree(scew_tree *g_tree, char *filename);

#endif // __OMA_DMT_H__
