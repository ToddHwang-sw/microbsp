#ifndef __STORAGE_HEADER__
#define __STORAGE_HEADER__

#ifdef __cplusplus
extern "C" {
#endif

enum {
	STOR_FILE = 0,
	STOR_MTD,
	STOR_MAX
};

#define TEMP_XML_PREFIX "/var/tmp"
#define DEFAULT_DEVTYPE STOR_FILE
#define DEFAULT_DEVNAME TEMP_XML_PREFIX "/xml.data"

/*
 *  S T O R A G E      L A Y O U T     F O R     C O N F I G U R A T I O N
 *
 *
 *  The size of Header = 128Kbytes (general NAND flash erase block size)
 *  The size of Data   = 2Mbytes   (customizable with other values greater than 128Kbytes)
 *
 *  +--------------------------------------------------+  <--+   <-- 0
 *  |                                                  |     |
 *  |      HEADER #1    ( 128Kbytes )                  |     |
 *  |                                                  |     |
 *  +--------------------------------------------------+     |
 *  |                                                  |     |
 *  |      HEADER #2    ( 128Kbytes )                  |     |
 *  |                                                  |   Just one of 4 headers
 *  +--------------------------------------------------+   is useful.
 *  |                                                  |
 *  |      HEADER #3    ( 128Kbytes )                  |     |
 *  |                                                  |     |
 *  +--------------------------------------------------+     |
 *  |                                                  |     |
 *  |      HEADER #4    ( 128Kbytes )                  |     |
 *  |                                                  |     |
 *  +--------------------------------------------------+  <--+   <-- STORAGE_DATA_START
 *  |                                                  |     |
 *  |      DATA BLOCK  #1 ( 1Mbytes )                  |     |
 *  |                                                  |   Two Data blocks are simplely
 *  +--------------------------------------------------+   switched every write...
 *  |                                                  |
 *  |      DATA BLOCK  #2 ( 1Mbytes )                  |     |
 *  |                                                  |     |
 *  +--------------------------------------------------+  <--+   <-- STORAGE_SIZE
 *
 *
 *  HEADER  MANAGEMENT
 *
 *      1. If #N block is physically OK (not bad block in NAND), use this.
 *      2. If update of #N block happens, it will be done in #N+1 and then
 *         switch to #N+1. If #N+1 block is physically bad, the scan will
 *         go on to the next #N+2.
 *      3. When switching happens, the field of "rsize" in old one will be
 *         zeroed to make user identify which is the latest.
 *      4. Next header block selection and Data block switching happens always
 *         together at every data update.
 *
 */

/*
 * These paramateres have been disciplined from the general layout of NAND flash memory.
 * NAND flash may have intrinscal bad page and its size is usually 128kbytes (erase page size).
 * Depending on H/W layout, it may change even to 256Kbytes but we use 128Kbytes under experiences
 * from the previous cases. STORAGE_HEADER_NUM is the chance of looking for safe block by bypassing
 * bad page.
 */
#define _KBYTES(x)  ((x)*(1024))
#define _MBYTES(x)  _KBYTES(((x)*(1024)))

#define STORAGE_IO_SIZE     _KBYTES(128)

/* storage header */
#define STORAGE_HEADER_SIZE STORAGE_IO_SIZE

/* placed in the beginning of STORAGE_IO_SIZE */
typedef union {
	struct {
		/* Tragedy (2018/11/08) - 13 young people was dead in a bar in L.A. */
		#define STORAGE_HEADER_MAGIC        0xdeadb0d1
		unsigned int magic;     /* magic code   */
		unsigned int dirty;     /* dirty flag   */
		unsigned short crc;     /* 16bit CRC    */
		#define STOR_FLAG_CLEAN     0x0000
		#define STOR_FLAG_ONDIRTY   0x00FF
		#define STOR_FLAG_RECOVERED 0x2000
		#define STOR_FLAG_TOUCHED   0x4000
		#define STOR_FLAG_TBU       0x8000
		unsigned short flags;   /* flag */
		unsigned int start;     /* start offset in storage media */
		unsigned int rsize;     /* actual size of config */
	} __attribute__((packed)) block;
	unsigned char dummy[ STORAGE_HEADER_SIZE ];
} __attribute__((packed)) storage_header;

#define DEFAULT_STORAGE_HEADER_NUM  4
#define DEFAULT_STORAGE_DATA_NUM    2
#define DEFAULT_STORAGE_DATA_SIZE   _MBYTES(1)

/* configurable parameter */
typedef struct {
	unsigned int hnum;  /* header number */
	unsigned int bnum;  /* data block number */
	unsigned int bsize; /* data block size */
}__attribute__((aligned)) storage_info;

extern void xml_storage_select( int );
extern void xml_storage_deselect( void );
extern int  xml_storage_open( char *, char *, storage_info * );/* media file --> temporary data structure */
extern char * xml_storage_validate( char * );   /* temporary data structure --> temporary XML file ||
                                                    "xmlfn"   ----------------> temporary XML file */
extern void xml_storage_close( void );
extern int  xml_storage_update( char *, char * );

#ifdef __cplusplus
}
#endif

#endif /* __STORAGE_HEADER__ */
