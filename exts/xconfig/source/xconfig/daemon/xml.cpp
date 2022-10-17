#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>
#include <pthread.h>
#include <limits.h>
#include "scew.h"
#include "dmt.h"
#include "jansson.h"
#include "malloc.h"
#include "xmlmem.h"
#include "common.h"
#include "xml.h"

//#define dump_list_json_PRINT
static void dump_list_json(scew_list * node_list, json_t *json_out, int depth)
{
	#define DELIMITER "/"
	scew_element *node = (scew_element *)scew_list_data (node_list);
	scew_list *children_list = (scew_list *)scew_element_children (node);
	#ifdef dump_list_json_PRINT
	static char print_buffer[2048] = {0,};
	#endif  /* .._PRINT */

	if (children_list == NULL)
	{
		char *name = (char *)scew_element_name (node);
		char *contents = (char *)scew_element_contents (node);

		if (contents == NULL)
			depth--;

	#ifdef dump_list_json_PRINT
		if (depth)
			printf("%s%s%s=%s\n", print_buffer, DELIMITER, name, contents ? contents : "");
		else
			printf("%s=%s\n",name, contents ? contents : "");
	#endif  /* .._PRINT */

		/* json */
		if( json_out )
			json_object_set_new( json_out, name, json_string(contents) );
		return;
	}

	while(children_list != NULL)
	{
		scew_element *child = (scew_element *)scew_list_data (children_list);
		char *child_name = (char *)scew_element_name (child);
		char *child_contents = (char *)scew_element_contents (child);
		scew_list *sub_child_list = (scew_list *)scew_element_children (child);
		json_t *jobj = NULL;

		if (child_contents == NULL) {
			if (sub_child_list != NULL) {
	#ifdef dump_list_json_PRINT
				if (depth)
					strcat(print_buffer,DELIMITER);
				strcat(print_buffer,child_name);
	#endif  /* .._PRINT */

				if( json_out ) {
					jobj = json_object();
					json_object_set_new( json_out, child_name, jobj );
				}
			}
			dump_list_json(children_list, jobj, depth + 1);
		} else {
			dump_list_json(children_list, json_out, depth);
		}
		children_list = scew_list_next (children_list);
	}

	#ifdef dump_list_json_PRINT
	/* shrink down string */
	{
		char * p = strrchr(print_buffer,'/');
		if (!p)
			p = print_buffer;
		*p = 0x0; /* truncate */
	}
	#endif  /* .._PRINT */
}

void config_xml_remove(json_t *jml)
{
	void *tmp;
	const char *key;
	json_t *obj;

	json_object_foreach_safe(jml,tmp,key,obj){
		if (json_object_size(obj))
			config_xml_remove(obj);
		json_object_del(jml,key);
	}
}

/* key... */
json_t * config_xml_retrieve( scew_tree * tree, const char * key )
{
	json_t * jres = NULL;
	scew_list *list = NULL;

	if (!tree || !key)
		return NULL;

	/* json object */
	jres = json_object();
	if (!jres)
		return NULL;

	/* result list */
	list = config_xml_node_list_by_uri(tree, key);
	if (list == NULL ) {
		ERR("config_xml_node_list_by_uri(%s)::failed\n",key);
		json_decref(jres);
		return NULL;
	}

	/* build up json string */
	dump_list_json(list, jres, 0);

	/* clean up scew list */
	scew_list_free( list );

	return jres;
}

json_t * config_xml_set( scew_tree * tree, const char * key, const char *value )
{
	json_t * jres = NULL;
	scew_list *list = NULL;
	scew_element *elem;

	if (!tree || !key)
		return NULL;

	/* json object */
	jres = json_object();
	if (!jres)
		return NULL;

	/* result list */
	list = config_xml_node_list_by_uri(tree, key);
	if (list == NULL ) {
		ERR("config_xml_node_list_by_uri(%s)::failed\n",key);
		json_decref(jres);
		return NULL;
	}

	elem = (scew_element *)scew_list_data(list);
	if (elem) {
		if (scew_element_children(elem) != NULL) {
			ERR("config_xml_set(url=%s)not signle node\n",key);
			return NULL;
		}
		if (!scew_element_set_contents(elem, value)) {
			ERR("config_xml_set(url=%s, value=%s)set_contents_error\n",
			    key,value);
			return NULL;
		}
		/* SUCCESS !! */
	}

	/* build up json string */
	dump_list_json(list, jres, 0 );

	/* clean up scew list */
	scew_list_free( list );

	return jres;
}

/*
 *
 * only single pair of key and value can be parsed.
 * More than two elements of json object cannot be handled yet. - Nov/05/2018
 *
 */
int config_xml_json_value(json_t *jobj, char *res)
{
	char * out = NULL;
	char *p, *x;

	if (!jobj || !res)
		goto __exit_config_json_value;

	out = json_dumps(jobj, JSON_PRESERVE_ORDER);
	if (!out)
		goto __exit_config_json_value;

	/* start scanning... */
	if (!((out[0] == '{') && (out[1] == '"')))
		goto __exit_config_json_value;

	/* tokenize key part */
	p = (char *)strchr((const char *)&out[2],'"');
	if (    !(*p == '"' &&
	          *(p+1) == ':' &&
	          *(p+2) == ' ' &&
	          *(p+3) == '"')   )
		goto __exit_config_json_value;
	*p = 0x0; /* null termination */
	/* tokenize value part */
	x = (char *)strchr(p+4, '"');
	*x = 0x0; /* null termination */
	strcpy(res,p+4);

	__os_free(out);
	return 0;

__exit_config_json_value:
	if (out)
		__os_free(out);
	return 1;
}

int config_xml_write_tree( scew_tree * tree, char *xmlf )
{
	/* Writing to a file... */
	unlink( xmlf );

	if (!tree || !xmlf)
		return 1;

	if( scew_writer_tree_file( tree, xmlf ) == false )
		ERR("scew_writer_tree_file::failed\n");

	return 0;
}

/*
 *
 *
 * X M L    T R E E   M E R G E
 *
 *
 */

static scew_list *list_for_merge = NULL;

/*
 * Merge to new configuration
 */
static bool config_xml_merge_write( scew_tree *dest_tree, scew_list *node_list, char* contents, char* file)
{
	scew_element *node = (scew_element *)scew_list_data(node_list);
	scew_list *children_list = (scew_list *)scew_element_children(node);
	//int n_attributes = 0; /*Currently not used */

	if (children_list == NULL) {
		if (contents != NULL) {
			//DBG("%s=%s\n", scew_element_name(node),contents);
			scew_element_set_contents(node, contents);
		} else {
			//DBG("%s=\n", scew_element_name(node));
			scew_element_free_contents(node);
		}

		if(file && !config_xml_write_tree(dest_tree, file))
			return false;
	}

	return true;
}

static char* get_node_path(scew_element *node)
{
	static char rpath[PATH_MAX];
	scew_element *parent = (scew_element *)node;

	memset(rpath, 0, PATH_MAX);

	while(parent)
	{
		char *name = (char *)scew_element_name(parent);
		char tmp[PATH_MAX];

		memset(tmp,0,PATH_MAX);
		strcpy(tmp,rpath);

		strcpy(rpath,"/");
		strcat(rpath,name);
		strcat(rpath,tmp);

		parent = (scew_element *)scew_element_parent(parent);
	}

	return rpath;
}

static scew_list* find_nodes(scew_element *node, char *find_path)
{
	char *path = (char *)get_node_path(node);

	if ( !strcmp(find_path, path) || !strcmp(find_path, "*") ) {
		if ( list_for_merge == NULL ) {
			list_for_merge = scew_list_create(node);
		} else {
			list_for_merge = scew_list_append(list_for_merge, node);
		}
	}

	scew_list *children_list = (scew_list *)scew_element_children(node);
	while(children_list != NULL)
	{
		scew_element *child = (scew_element *)scew_list_data(children_list);
		list_for_merge = (scew_list *)find_nodes(child, find_path);
		children_list = (scew_list *)scew_list_next(children_list);
	}
	scew_list_free(children_list);

	return list_for_merge;
}

static void config_xml_merge_tree(scew_list* node_list, scew_tree *dest_tree)
{
	scew_element *node = (scew_element *)scew_list_data(node_list);
	scew_list *children_list = (scew_list *)scew_element_children(node);
	scew_list *dest_node_list = NULL;
	char path[PATH_MAX] = {0};
	char *xpath;

	if (children_list == NULL)
	{
		char *contents = (char *)scew_element_contents(node);
		//char *name = (char *)scew_element_name(node);  /* Currently not used */

		sprintf(path, "%s", get_node_path(node));
		xpath = (char *)&(path[ strlen(XML_TOPLVL_NAME) ]);
		//DBG("node path = (%s) \n",xpath);

		dest_node_list = config_xml_node_list_by_uri(dest_tree, xpath);
		if(dest_node_list)
		{
			scew_element *dest_node = (scew_element *)scew_list_data(dest_node_list);
			char *dest_contents = (char *)scew_element_contents(dest_node);

			//DBG("%s=%s (%s)", name, contents, dest_contents);

			if (contents != NULL) {
				if ( (dest_contents != NULL) &&
				     (!strcmp(contents, dest_contents) || !strcmp(xpath, "/date"))) {
					config_xml_merge_write(dest_tree, dest_node_list, dest_contents, 0);
				} else {
					int n_attributes = scew_element_attribute_count(dest_node);
					if ( n_attributes > 0 ) {
						scew_element_delete_attribute_all(dest_node);
						config_xml_merge_write(dest_tree, dest_node_list, dest_contents, 0);
					} else {
						config_xml_merge_write(dest_tree, dest_node_list, contents, 0);
					}
				}
				__os_free(dest_node_list);
			}
		}
	}

	while(children_list != NULL)
	{
		config_xml_merge_tree(children_list, dest_tree);
		children_list = (scew_list *)scew_list_next(children_list);
	}
	scew_list_free(children_list);
}

int config_xml_merge(char* oxml, char* nxml)
{
	scew_tree   *otree = NULL;
	scew_parser *oparser = NULL;
	scew_tree   *ntree = NULL;
	scew_parser *nparser = NULL;
	scew_list   *nlist = NULL;
	int ret;

	/* reading old tree */
	ret = config_xml_init(oxml, &oparser, &otree);
	if ( ret < 0 ) {
		ERR("Fail to load cmnnv configuration xml for merge !!!");
		return -1;
	}

	/* reading new tree */
	ret = config_xml_init(nxml, &nparser, &ntree);
	if ( ret < 0 ) {
		ERR("Fail to load source configuration xml for merge !!!");
		ret = -1;
		goto out;
	}

	/* check version */
	{
		/* old list product */
		char * over = (char *)scew_tree_xml_version(otree);
		char * nver = (char *)scew_tree_xml_version(ntree);

		DBG("VERSION (%s vs %s) \n",over, nver);

		if (strcasecmp(nver,over) <= 0) {
			DBG("CURRENTLY LATEST VERSION\n");
			ret = -1;
			goto out;
		}
	}

	/* check product */
	{
		/* old list product */
		scew_list * olist = (scew_list *)config_xml_node_list_by_uri(otree, "product");
		if (olist == NULL ) {
			ERR("config_xml_node_list_by_uri(%s)::failed\n","product");
			ret = -1;
			goto out;
		}
		char * oprod = (char *)scew_element_contents( (scew_element *)scew_list_data(olist) );

		scew_list * nlist = (scew_list *)config_xml_node_list_by_uri(ntree, "product");
		if (nlist == NULL ) {
			ERR("config_xml_node_list_by_uri(%s)::failed\n","product");
			ret = -1;
			goto out;
		}
		char * nprod = (char *)scew_element_contents( (scew_element *)scew_list_data(nlist) );

		DBG("PRODUCT (%s vs %s) \n",oprod, nprod);

		if (strcasecmp(oprod,nprod)) {
			ERR("Different product\n");
			ret = -1;
			goto out;
		}
	}

	/* check date */
	{
		/* old list product */
		scew_list * olist = (scew_list *)config_xml_node_list_by_uri(otree, "date");
		if (olist == NULL ) {
			ERR("config_xml_node_list_by_uri(%s)::failed\n","date");
			ret = -1;
			goto out;
		}
		char * odate = (char *)scew_element_contents( (scew_element *)scew_list_data(olist) );

		scew_list * nlist = (scew_list *)config_xml_node_list_by_uri(ntree, "date");
		if (nlist == NULL ) {
			ERR("config_xml_node_list_by_uri(%s)::failed\n","date");
			ret = -1;
			goto out;
		}
		char * ndate = (char *)scew_element_contents( (scew_element *)scew_list_data(nlist) );

		DBG("DATE (%s vs %s) \n",odate, ndate);

		if (strcasecmp(ndate,odate) <=  0) {
			ERR("CURRENTLY LATEST VERSION DATE\n");
			ret = -1;
			goto out;
		}
	}

	list_for_merge = NULL;
	nlist = find_nodes(scew_tree_root(otree), (char *)XML_TOPLVL_NAME);
	if (nlist == NULL) {
		ERR("Cannot find path for add new element.");
		ret = -1;
		goto out;
	}

	config_xml_merge_tree(nlist, ntree);
	config_xml_write_tree(ntree, oxml);

	ret = 0;
out:
	if (oparser || otree) config_xml_exit(oparser, otree);
	if (nparser || ntree) config_xml_exit(nparser, ntree);
	return ret;
}


