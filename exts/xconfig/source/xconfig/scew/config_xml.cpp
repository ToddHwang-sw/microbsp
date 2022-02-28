#include <errno.h>
#include <limits.h>
#include <math.h>

#include "element.h"
#include "tree.h"
#include "list.h"
#include "str.h"
#include "parser.h"
#include "dmt.h"
#include "error.h"
#include "base64.h"
#include "writer.h"

int config_xml_init(XML_Char *xml_file, scew_parser **g_parser, scew_tree **g_tree)
{
	scew_parser *parser = scew_parser_create();
	scew_tree *tree;

	*g_parser = parser;
	scew_parser_ignore_whitespaces(parser, 1);

	if (!scew_parser_load_file(parser, xml_file)) {
		scew_error code = scew_error_code();
		fprintf(stderr,"Unable to load file (error #%d: %s)", code, scew_error_string(code));
		if (code == scew_error_expat) {
			enum XML_Error expat_code = scew_error_expat_code(parser);

			fprintf(stderr,"Exapt error #%d (line %d, column %d): %s", expat_code,
			        scew_error_expat_line(parser),
			        scew_error_expat_column(parser),
			        scew_error_expat_string(expat_code));
		}
		if (parser) {
			scew_parser_free(parser);
			*g_parser = NULL;
		}
		return -1;
	}

	tree = scew_parser_tree(parser);
	*g_tree = tree;

	return 0;
}

scew_list *config_xml_node_list_by_uri(scew_tree *g_tree, const XML_Char *uri)
{
	scew_element *root;
	scew_list *list = NULL;

	if (g_tree == NULL || scew_tree_root(g_tree) == NULL) return NULL;

	if (!scew_strncmp(uri, _XT("/"XML_ROOT_NAME), 2))
		uri += XML_ROOT_NAME_LEN + 1;

	root = scew_tree_root(g_tree);
	if (root == NULL)
		return NULL;

	if (!scew_strcmp(uri, _XT(".")))
		return list;

	return node_list_by_uri(root, uri);
}

void config_xml_exit(scew_parser *g_parser, scew_tree *g_tree)
{
	if (g_tree) {
		scew_tree_free(g_tree);
		g_tree = NULL;
	}
	if (g_parser) {
		scew_parser_free(g_parser);
		g_parser = NULL;
	}
}

int config_write_tree(scew_tree *g_tree, char *filename)
{
	if (g_tree)
		return (int)scew_writer_tree_file(g_tree, filename);

	return -1;
}

/*
 * Merge to new configuration
 */
bool config_merge_write( scew_tree *dest_tree, scew_list *node_list, char* contents, char* file)
{
	scew_element *node = (scew_element *)scew_list_data(node_list);
	scew_list *children_list = scew_element_children(node);
	//int n_attributes = 0; /*Currently not used */

	if (children_list == NULL) {
		if (contents != NULL) {
			//DBG("%s=%s", name, contents);
			printf("<%s>", contents);
			scew_element_set_contents(node, contents);
		} else {
			//DBG("%s=\n", name);
			printf("<%s>=\n",__func__);
			scew_element_free_contents(node);
		}

		if(file && !config_write_tree(dest_tree, file))
			return false;
	}

	return true;
}

char* get_node_path(scew_element *node)
{
	static char result_path[1024];

	scew_element *parent = node;

	memset(result_path, 0, 1024);//sprintf(result_path, "");

	while(parent)
	{
		char *name = (char *)scew_element_name(parent);
		char tmp[1024];

		sprintf(tmp, "%s", result_path);

		sprintf(result_path, "%s", "/");
		sprintf(result_path, "%s%s", result_path, name);
		sprintf(result_path, "%s%s", result_path, tmp);

		parent = scew_element_parent(parent);
	}

	return result_path;
}

static scew_list *result_list = NULL;
void find_nodes_reset()
{
	result_list = NULL;
}

scew_list* find_nodes(scew_element *node, char *find_path)
{
	char *path = get_node_path(node);

	if ( !strcmp(find_path, path) || !strcmp(find_path, "*") ) {
		if ( result_list == NULL ) {
			result_list = scew_list_create(node);
		} else {
			result_list = scew_list_append(result_list, node);
		}
	}

	scew_list *children_list = scew_element_children(node);
	while(children_list != NULL)
	{
		scew_element *child = (scew_element *)scew_list_data(children_list);
		result_list = (scew_list *)find_nodes(child, find_path);
		children_list = scew_list_next(children_list);
	}
	scew_list_free(children_list);

	return result_list;
}

void config_merge_tree(scew_list* node_list, scew_tree *dest_tree)
{
	scew_element *node = (scew_element *)scew_list_data(node_list);
	scew_list *children_list = scew_element_children(node);
	scew_list *dest_node_list = NULL;
	char path[1024] = {0};

	if (children_list == NULL)
	{
		char *contents = (char *)scew_element_contents(node);
		char *name = (char *)scew_element_name(node);  /* Currently not used */

		sprintf(path, "%s", get_node_path(node));

		dest_node_list = config_xml_node_list_by_uri(dest_tree, path);
		if(dest_node_list)
		{
			scew_element *dest_node = (scew_element *)scew_list_data(dest_node_list);
			char *dest_contents = (char *)scew_element_contents(dest_node);

			//DBG("[%s] %s=%s (%s)", __FUNCTION__, name, contents, dest_contents);
			printf("[%s] %s=%s (%s)", __FUNCTION__, name, contents, dest_contents);

			if (contents != NULL) {
				if ( (dest_contents != NULL) &&
				     (!strcmp(contents, dest_contents) || !strcmp(path, "/config/date"))) {
					config_merge_write(dest_tree, dest_node_list, dest_contents, 0);
				} else {
					int n_attributes = scew_element_attribute_count(dest_node);
					if ( n_attributes > 0 ) {
						scew_element_delete_attribute_all(dest_node);
						config_merge_write(dest_tree, dest_node_list, dest_contents, 0);
					} else {
						config_merge_write(dest_tree, dest_node_list, contents, 0);
					}
				}
				free(dest_node_list);
			}
		}
	}

	while(children_list != NULL)
	{
		config_merge_tree(children_list, dest_tree);
		children_list = scew_list_next(children_list);
	}
	scew_list_free(children_list);
}

int config_merge_to_xml(char* old_cfg, char* new_cfg)
{
	scew_tree *old_tree = NULL;
	scew_parser *old_parser = NULL;
	scew_tree *new_tree = NULL;
	scew_parser *new_parser = NULL;
	scew_list *node_list = NULL;
	int ret;

	ret = config_xml_init(old_cfg, &old_parser, &old_tree);
	if ( ret < 0 ) {
		fprintf(stderr,"Fail to load cmnnv configuration xml for merge !!!");
		return -1;
	}

	ret = config_xml_init(new_cfg, &new_parser, &new_tree);
	if ( ret < 0 ) {
		fprintf(stderr,"Fail to load source configuration xml for merge !!!");
	}

	//Add new element
	find_nodes_reset();
	node_list = find_nodes(scew_tree_root(old_tree), (char *)"/date");
	if (node_list == NULL) {
		fprintf(stderr,"Cannot find path for add new element.");
		goto out;
	}
	config_merge_tree(node_list, new_tree);

	config_write_tree(new_tree, old_cfg);

out:
	config_xml_exit(old_parser, old_tree);
	config_xml_exit(new_parser, new_tree);

	return 0;
}


