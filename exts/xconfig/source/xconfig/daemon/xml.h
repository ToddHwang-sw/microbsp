#ifndef __XML_MANIP_HEADER__
#define __XML_MANIP_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * XML should be
 *
     <?xml version="1.1.0.1" encoding="utf-8">
     <config>
         <product>RRH</product>
         <date>201811300830</date>
         ...
         ...
     </config>
 *
 *   Both <product> and <date> should exist inside of <config>.
 *
 */
#define XML_TOPLVL_NAME "/config"

extern json_t * config_xml_retrieve( scew_tree *, const char * );
extern json_t * config_xml_set( scew_tree *, const char *, const char * );
extern int config_xml_json_value( json_t *, char * );
extern void config_xml_remove(json_t *);
extern int config_xml_write_tree( scew_tree *, char * );
extern int config_xml_merge(char*, char* );

#ifdef __cplusplus
}
#endif

#endif

