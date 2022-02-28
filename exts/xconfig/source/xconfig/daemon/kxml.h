#ifndef __XML_DEFINE__
#define __XML_DEFINE__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * GCT UCFG XML ROOT NODE NAME
 */
#define PATH_PREFIX "/config/"

struct XmlData {
	char *path;
	char *value;
	char *newval;
	unsigned int loc;
	struct XmlData *next;
};

/* XML header parsing */
extern char * kxml_header( char *start );
extern struct XmlData * kxml_parse( char *data, int verbose );
extern void kxml_print( struct XmlData * );
extern void kxml_free( struct XmlData * );

#ifdef __cplusplus
}
#endif

#endif /* __XML_DEFINE__ */
