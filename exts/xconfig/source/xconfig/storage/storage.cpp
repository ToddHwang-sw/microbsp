#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mtd/mtd-user.h>
#include <pthread.h>
#include "common.h"
#include "malloc.h"
#include "xmlmem.h"
#include "storage.h"
#include "kxml.h"

struct storage;

struct storage {
	void *info;
	void *(*stor_select)(char *);
	int (*stor_deselect)(struct storage *);
	int (*stor_size)(struct storage *);
	int (*stor_rewind)(struct storage *, int);              /* seek */
	int (*stor_open)(struct storage *);                     /* storage ready */
	int (*stor_write)(struct storage *,char *,int);         /* storage write */
	int (*stor_read)(struct storage *,char *,int);          /* storage read */
	int (*stor_close)(struct storage *);                    /* storage close */
} __attribute__((aligned));

/* stor_open */
enum {
	STOR_OPEN_SUCCESS = 0,
	STOR_OPEN_FAILURE,
	STOR_OPEN_CREATED
};

/* stor_write */
enum {
	STOR_WRITE_SUCCESS = 0,
	STOR_WRITE_FAILURE,
	STOR_WRITE_SYSERR,
};

enum {
	STOR_READ_SUCCESS = 0,
	STOR_READ_FAILURE,
	STOR_READ_SYSERR,
};

/*
 *
 *
 * P R I V A T E     F I L E     I N T E R F A C E
 *
 *
 */

/* file information */
typedef struct {
	char  *name;    /* file name */
	int fd;         /* file descriptor */
	int size;       /* file size */
	int amount;     /* temporary */
	void  *header;  /* header */
}file_stor_t;

static void * file_select(char *name)
{
	file_stor_t *fs;

	/* file info */
	fs = (file_stor_t *)__os_malloc( sizeof(file_stor_t) );
	if (!fs)
		return NULL;

	/* name allocation */
	fs->name = (char *)__os_malloc( strlen(name)+1 );
	if (!fs->name) {
		__os_free( fs );
		return NULL;
	}
	memset( fs->name, 0, strlen(name)+1 );
	memcpy( fs->name, name, strlen(name) );

	return (void *)fs;
}

static int file_deselect(struct storage *stor)
{
	file_stor_t *fs;

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	fs = (file_stor_t *)stor->info;

	if (fs) {
		if (fs->name)
			__os_free( fs->name );
		__os_free( fs );
	}

	stor->info = NULL;
	return 0;
}

static int file_size(struct storage *stor)
{
	file_stor_t *fs;
	struct stat st;

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	fs = (file_stor_t *)stor->info;

	if (stat(fs->name,&st) < 0)
		return 0;

	return (int)st.st_size;
}

static int file_rewind(struct storage *stor, int off)
{
	file_stor_t *fs;
	int loc;

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	fs = (file_stor_t *)stor->info;

	/* sidekick menu - current location */
	if (off == -1) {
		loc = (int)lseek(fs->fd, (off_t)0, SEEK_CUR);
		if (loc < 0)
			return -1;
	} else {
		loc = lseek(fs->fd, (off_t)off, SEEK_SET);
		if (loc < 0)
			return -1;
	}

	return loc;
}

static int file_open(struct storage *stor)
{
	file_stor_t *fs;

	if (!stor)
		return STOR_OPEN_FAILURE;

	if (!stor->info)
		return STOR_OPEN_FAILURE;

	fs = (file_stor_t *)stor->info;

	fs->fd = open(fs->name,O_RDWR);
	if (fs->fd < 0) {
		fs->fd = open(fs->name,O_RDWR|O_CREAT,0644);
		return (fs->fd <= 0) ?  STOR_OPEN_FAILURE : STOR_OPEN_CREATED;
	} else
		return STOR_OPEN_SUCCESS;
}

static int file_write(struct storage *stor, char *buffer, int size)
{
	file_stor_t *fs;

	if (!stor)
		return STOR_WRITE_SYSERR;

	if (!stor->info)
		return STOR_WRITE_SYSERR;

	fs = (file_stor_t *)stor->info;

	fs->amount = write(fs->fd, buffer, size);
	if (fs->amount < 0)
		return STOR_WRITE_SYSERR;

	return (fs->amount == size) ? STOR_WRITE_SUCCESS : STOR_WRITE_FAILURE;
}

static int file_read(struct storage *stor, char *buffer, int size)
{
	file_stor_t *fs;

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	fs = (file_stor_t *)stor->info;

	fs->amount = read(fs->fd, buffer, size);
	if (fs->amount < 0)
		return STOR_READ_SYSERR;

	return (fs->amount == size) ? STOR_READ_SUCCESS : STOR_READ_FAILURE;
}

static int file_close(struct storage *stor)
{
	file_stor_t *fs;

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	fs = (file_stor_t *)stor->info;

	close(fs->fd);

	return 0;
}

/*
 *
 *
 * P R I V A T E     M T D       I N T E R F A C E
 *
 *
 */

/* file information */
typedef struct {
	char  *name;    /* partition name */
	int fd;
	struct mtd_info_user mtd;
	void  *header;  /* header */
}mtd_stor_t;

static void * mtd_select(char *name)
{
	mtd_stor_t *ms;

	/* file info */
	ms = (mtd_stor_t *)__os_malloc( sizeof(mtd_stor_t) );
	if (!ms)
		return NULL;

	/* name allocation */
	ms->name = (char *)__os_malloc( strlen(name)+1 );
	if (!ms->name) {
		__os_free( ms );
		return NULL;
	}
	memset( ms->name, 0, strlen(name)+1 );
	memcpy( ms->name, name, strlen(name) );

	ms->fd = open(ms->name,O_RDWR);
	if (ms->fd < 0) {
		__os_free( ms->name );
		__os_free( ms );
		return NULL;
	}

	if (ioctl(ms->fd, MEMGETINFO, &ms->mtd)) {
		__os_free( ms->name );
		__os_free( ms );
		close(ms->fd);
		return NULL;
	}
	close(ms->fd);

	/* check validity */
	if (ms->mtd.erasesize > STORAGE_IO_SIZE) {
		__os_free( ms->name );
		__os_free( ms );
		ERR("MTD:: device erase size is not %dKbytes but %dKbytes.\nWe cannot use this MTD partition.\n",
		    STORAGE_IO_SIZE >> 10, ms->mtd.erasesize >> 10 );
		return NULL;
	}

	return (void *)ms;
}

static int mtd_deselect(struct storage *stor)
{
	mtd_stor_t *ms;

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	ms = (mtd_stor_t *)stor->info;

	if (ms) {
		if (ms->name)
			__os_free( ms->name );
		__os_free( ms );
	}

	stor->info = NULL;
	return 0;
}

static int mtd_size(struct storage *stor)
{
	mtd_stor_t *ms;

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	ms = (mtd_stor_t *)stor->info;

	return (int)ms->mtd.size;
}

static int mtd_rewind(struct storage *stor, int off)
{
	mtd_stor_t *ms;
	int loc;

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	ms = (mtd_stor_t *)stor->info;

	/* sidekick menu - current location */
	if (off == -1) {
		loc = (int)lseek(ms->fd, (off_t)0, SEEK_CUR);
		if (loc < 0)
			return -1;
	} else {
		if (off % STORAGE_IO_SIZE) {
			ERR("MTD: device rewind is not multiple of %dKbytes but %x\n",
			    STORAGE_IO_SIZE >> 10, off);
			return -1;
		}
		loc = lseek(ms->fd, (off_t)off, SEEK_SET);
		if (loc < 0)
			return -1;
	}

	return loc;
}

static int mtd_open(struct storage *stor)
{
	mtd_stor_t *ms;

	if (!stor)
		return STOR_OPEN_FAILURE;

	if (!stor->info)
		return STOR_OPEN_FAILURE;

	ms = (mtd_stor_t *)stor->info;

	ms->fd = open(ms->name,O_RDWR);
	if (ms->fd < 0)
		return STOR_OPEN_FAILURE;
	else {
		return STOR_OPEN_SUCCESS;
	}
}

static int mtd_write(struct storage *stor, char *buffer, int size)
{
	mtd_stor_t *ms;
	int amount;
	int res;
	loff_t pivot;
	struct erase_info_user erase;

	if (!stor)
		return STOR_WRITE_SYSERR;

	if (!stor->info)
		return STOR_WRITE_SYSERR;

	ms = (mtd_stor_t *)stor->info;

	pivot = (loff_t)mtd_rewind( stor, -1 );
	erase.start = pivot;
	erase.length = ms->mtd.erasesize;

	/* programming */
	amount = 0;
	while( amount < size ) {
		loff_t off = erase.start;

		if( (int)(off-pivot) >= (int)ms->mtd.size ) {
			ERR("MTD: programming out of range %x:%x \n", (int)(off - pivot), ms->mtd.size );
			break;
		}

		if( ioctl(ms->fd, MEMGETBADBLOCK, &off) > 0 ) {
			DBG("MTD: programming bad block %x\n", (int)off);
			erase.start += erase.length;
			/* Skipping BAD BLOCK */
			lseek(ms->fd, (off_t)ms->mtd.erasesize, SEEK_CUR);
			continue;
		}

		/* erase */
		if( ioctl(ms->fd, MEMERASE, &erase ) < 0) {
			ERR("MTD: programming erase error %08x\n", erase.start);
			return STOR_WRITE_FAILURE;
		}

		/* write */
		res = write(ms->fd, buffer + amount, ms->mtd.erasesize );
		if (res < 0) {
			ERR("MTD: programming error at %d/%d \n",amount,size);
			return STOR_WRITE_FAILURE;
		}

		if ((int)res != (int)ms->mtd.erasesize)
			DBG("MTD: programming %d != %d\n", res, ms->mtd.erasesize);

		amount += res;
		erase.start += res;
	}

	return STOR_WRITE_SUCCESS;
}

static int mtd_read(struct storage *stor, char *buffer, int size)
{
	mtd_stor_t *ms;
	int amount;
	int res;
	loff_t start, pivot;

	if (!stor)
		return STOR_READ_SYSERR;

	if (!stor->info)
		return STOR_READ_SYSERR;

	ms = (mtd_stor_t *)stor->info;

	pivot = (loff_t)mtd_rewind( stor, -1 );
	start = pivot;

	/* programming */
	amount = 0;
	while( amount < size ) {
		//int off = start;
		loff_t off = start;

		if( (int)(off-pivot) >= (int)ms->mtd.size ) {
			ERR("MTD: reading out of range %x:%x \n", (int)(off - pivot), ms->mtd.size );
			break;
		}

		if( ioctl(ms->fd, MEMGETBADBLOCK, &off) > 0 ) {
			DBG("MTD: reading bad block %x\n", (int)off);
			start += ms->mtd.erasesize;
			/* Skipping BAD BLOCK */
			lseek(ms->fd, (off_t)ms->mtd.erasesize, SEEK_CUR);
			continue;
		}

		/* write */
		res = read(ms->fd, buffer + amount, ms->mtd.erasesize );
		if (res < 0) {
			ERR("MTD: reading error at %d/%d \n",amount,size);
			return STOR_READ_FAILURE;
		}

		if ((int)res != (int)ms->mtd.erasesize)
			DBG("MTD: reading %d != %d\n", res, ms->mtd.erasesize);

		amount += res;
		start += res;
	}

	return STOR_READ_SUCCESS;
}

static int mtd_close(struct storage *stor)
{
	mtd_stor_t *ms;

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	ms = (mtd_stor_t *)stor->info;

	close(ms->fd);

	return 0;
}

/*
 *
 *
 * C O M M O N      S T O R A G E    I N T E R F A C E
 *
 *
 */

static struct storage stor_interface[STOR_MAX]=
{
	[STOR_FILE] = {
		.info       = NULL,
		.stor_select= file_select,
		.stor_deselect= file_deselect,
		.stor_size  = file_size,
		.stor_rewind= file_rewind,
		.stor_open  = file_open,
		.stor_write = file_write,
		.stor_read  = file_read,
		.stor_close = file_close
	},
	[STOR_MTD] = {
		.info       = NULL,
		.stor_select= mtd_select,
		.stor_deselect= mtd_deselect,
		.stor_size  = mtd_size,
		.stor_rewind= mtd_rewind,
		.stor_open  = mtd_open,
		.stor_write = mtd_write,
		.stor_read  = mtd_read,
		.stor_close = mtd_close
	},
};

/* XML storage information */
static
struct xml_storage {
	/* physical information */
	int hnum;       /* header number */
	int bnum;       /* block nunmber */
	int bsize;      /* block size */
	int dmin;       /* minimum data block */

	int size;
	storage_header header;   /* storage header  */
	char * data;             /* data block */
	unsigned int * headindex; /* header location */
	unsigned int * dataindex;   /* data location */
	int hindex;             /* header index */
	struct storage *stor;   /* current storage */
} __attribute__((aligned)) xmlstor = {0,};

#define STOR_HEADER_NUM(stor)   ((stor)->hnum)
#define STOR_DATA_NUM(stor)     ((stor)->bnum)
#define STOR_DATA_SIZE(stor)    ((stor)->bsize)
#define STOR_SIZE(stor)         \
	((STORAGE_HEADER_SIZE * STOR_HEADER_NUM(stor)) + (STOR_DATA_NUM(stor) * STOR_DATA_SIZE(stor)))
#define STOR_DATA_MIN(stor)     ((stor)->dmin)

/* REQUEST REBOOT CODE */
#define REQ_REBOOT 101

/* #define XTEST */
#ifdef XTEST
static int test_method = 0;
#endif

/*
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *
 * P R I V A T E                 I N T E R F A C E
 *
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 */
static __inline__ unsigned short _xml_storage_crc( char *data, int size )
{
	int adjsize;
	unsigned int total;
	unsigned short *u16p;

	adjsize = size & ~0x1; /* 16 bit align */

	total = 0;
	u16p = (unsigned short *)data;
	for(int ix = 0; ix < (adjsize / (int)sizeof(unsigned short)); ix++ )
		total += (unsigned short)(*u16p++);

	total = (total >> 16) + (total & 0xFFFF);
	total = (total >> 16) + total;
	total = ~total;

	return (unsigned short)total;
}

static int _xml_storage_clean( void )
{
	struct storage *stor = xmlstor.stor;
	int ret = 1;
	int hcount = 0;
	int dcount = 0;
	int nblock;
	storage_header * sh;
	static char blk[ STORAGE_IO_SIZE ] = {0,};

	if (!stor)
		return -1;

	if (!stor->info)
		return -1;

	if (stor->stor_open(stor) != STOR_OPEN_SUCCESS)
		return -1;

	/* jump to data location */
	if (stor->stor_rewind(stor, xmlstor.dataindex[ 0 ]) < 0) {
		ERR("storage rewind( DATA [%d] = [0x%08x] ) failed\n", 0, xmlstor.dataindex[ 0 ]);
		goto __exit_xml_storage_clean;
	}

	/* data writing first for map */
	for( int ix = 0; ix < STOR_DATA_NUM(&xmlstor); ix++ ) {
		for( int num = 0; num < (STOR_DATA_SIZE(&xmlstor)/STORAGE_IO_SIZE); num++ ) {
			if (stor->stor_write( stor,
			                      (char *)&blk,
			                      sizeof(blk)) != STOR_WRITE_SUCCESS) {
				ERR("stor_write(DATA..%d/%d)::failed\n",ix,num);
				goto __exit_xml_storage_clean;
			}
		}
		DBG("DATA  [ %-2d ] START = 0x%x / DATA SIZE = %d\n",
		    ix, stor->stor_rewind( stor, -1 ), STOR_DATA_SIZE(&xmlstor) );
		++dcount;
	}

	/* data block */
	if (dcount < 2) {
		ERR("\nDevice/File does not have simply %d data blocks\n",dcount);
		ERR("We cannot use this device/file for storage.\n\n");
		goto __exit_xml_storage_clean;
	}

	/* jump to header location */
	if (stor->stor_rewind(stor, 0) < 0) {
		ERR("storage rewind( 0 ) failed\n");
		goto __exit_xml_storage_clean;
	}

	DBG("Storage Header Size = %d \n", (int)sizeof(storage_header));

	sh = (storage_header *)&xmlstor.header;
	memset(sh->dummy, 0, sizeof(storage_header) );

	/* initial header value */
	sh->block.magic = htonl( STORAGE_HEADER_MAGIC );
	sh->block.dirty = 1; /* first one is only dirty */
	sh->block.crc = 0;
	sh->block.rsize = 0;

	nblock = 0;
	sh->block.start = xmlstor.dataindex[ nblock ];

	/* header writing */
	for( int ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
		if( stor->stor_rewind( stor,
		                       xmlstor.headindex[ ix ] ) < 0 ) {
			ERR("storage rewind( HEADER [%d] = [0x%08x] ) failed\n", ix, xmlstor.headindex[ ix ]);
			goto __exit_xml_storage_clean;
		}
		if( stor->stor_write( stor,
		                      (char *)sh,
		                      sizeof(storage_header)) != STOR_WRITE_SUCCESS
		    ) {
			ERR("stor_write(HEADER.. %dth)::failed\n",ix);
			continue;
		}
		DBG("HEADER[ %-2d ] START = 0x%x / DATA OFFSET = %d (%d) / DIRTY = %d \n",
		    ix, stor->stor_rewind( stor, -1 ), sh->block.start, nblock, sh->block.dirty );
		++hcount;
		if (hcount == 1) {
			sh->block.dirty = 0; /* clean up dirty */
			sh->block.flags = 0; /* clean up flags */
		}
		nblock = (nblock + 1) % STOR_DATA_NUM(&xmlstor);
		sh->block.start = xmlstor.dataindex[ nblock ];
	}

	/*
	 * THIS IS VERY IMPORTANT CHECK !!
	 *
	 * We SHOULD HAVE AT LEAST 2 HEADER BLOCKS AND 2 DATA BLOCKS FOR SWITCHING.!!
	 *
	 */
	if (hcount < 2) {
		ERR("\nDevice/File does not have simply %d header blocks\n", hcount);
		ERR("We cannot use this device/file for storage.\n\n");
		goto __exit_xml_storage_clean;
	}

	ret = 0;

__exit_xml_storage_clean:

	stor->stor_close(stor);
	return ret;
}

static int _xml_storage_header_fetch( struct storage *stor, int selection )
{
	storage_header *sh, tmp;

	/* storage header */
	sh = (storage_header *)&xmlstor.header;

	memset((char *)&tmp, 0, sizeof(storage_header) );

	/* header read */
	xmlstor.hindex = -1;
	for( int ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
		if (selection != (-1)) {
			if (selection != ix)
				continue; /* skip ... */
		}
		if (stor->stor_rewind(stor,
		                      xmlstor.headindex[ ix ] ) < 0) {
			ERR("storage rewind( HEADER [%d] = [0x%08x] ) failed\n", ix, xmlstor.headindex[ ix ]);
			return -1;
		}
		if (stor->stor_read(stor,
		                    (char *)&tmp,
		                    sizeof(storage_header)) != STOR_READ_SUCCESS) {
			ERR("stor_read(HEADER.. %dth)::failed\n",ix);
			continue;
		}
		if (selection != (-1)) {
			xmlstor.hindex = ix; /* current header index */
			memcpy( (char *)sh, (char *)&tmp, sizeof(storage_header) );
			return 0;
		}
		if (tmp.block.dirty) {
			if (xmlstor.hindex != -1) {
				ERR("MULTIPLE DIRTY BLOCK(%d - %d) FOUND FLAG=%04x ",ix, xmlstor.hindex, tmp.block.flags);
				xmlstor.hindex = -1; /* invalidate previous dirty block */
				return -1;
			}
			DBG("DIRTY HEADER[ %-2d ] START = 0x%x / DATA OFFSET = %d FLAG=%04x \n",
			    ix, stor->stor_rewind( stor, -1 ), tmp.block.start, tmp.block.flags );
			xmlstor.hindex = ix; /* current header index */
			memcpy( (char *)sh, (char *)&tmp, sizeof(storage_header) );
		}
	}

	if (xmlstor.hindex == (-1)) {
		ERR("HEADER NOT FOUND \n");
		return -1;
	}

	return 0;
}

static __inline__ int xml_storage_header_fixup_write( struct storage *stor, int hloc, char *hdata )
{
	if (stor->stor_rewind(stor,
	                      xmlstor.headindex[ hloc ] ) < 0) {
		ERR("storage rewind( HEADER [%d] = [0x%08x] ) failed\n", hloc, xmlstor.headindex[ hloc ]);
		return -1;
	}
	if (stor->stor_write(stor,
	                     (char *)hdata,
	                     sizeof(storage_header)) != STOR_WRITE_SUCCESS) {
		ERR("stor_write(HEADER.. %dth)::failed\n",hloc);
		return -1;
	}

	return 0;
}

static __inline__ int xml_storage_data_recover( struct storage *stor, storage_header *sh, int loc )
{
	static char *tmpblk, *ploc;
	int nblock;

	DBG("BLOCK[%d] HEADER - LATEST - NOTOK - PURELY BROKEN \n", loc);
	/* Parse data block .... */
	/* Calculate size & CRC */

	memset( (char *)sh, 0, sizeof(storage_header) ); /* clean~ */

	/* CALCULATING DATA START ADDRESS */
	nblock = 0;
	for( int iy = 0; iy < loc; iy++ )
		nblock = (nblock + 1) % STOR_DATA_NUM(&xmlstor);
	sh->block.start = xmlstor.dataindex[ nblock ];

	DBG("BLOCK[%d] HEADER DATA BEGINS AT %08x\n", loc, sh->block.start);
	if ( stor->stor_rewind( stor, sh->block.start ) < 0 ) {
		ERR("storage rewind( DATA [0x%08x] ) failed\n", sh->block.start);
		return -1;
	}
	tmpblk = (char *)__os_malloc( STOR_DATA_SIZE(&xmlstor) );
	if (!tmpblk) {
		ERR("__os_malloc(%d) error\n",STOR_DATA_SIZE(&xmlstor));
		return -1;
	}
	memset(tmpblk,0,STOR_DATA_SIZE(&xmlstor));
	DBG("BLOCK[%d] HEADER DATA READING START\n", loc);
	if ( stor->stor_read( stor,
	                      tmpblk,
	                      STOR_DATA_SIZE(&xmlstor) ) != STOR_READ_SUCCESS ) {
		ERR("stor_read( .. %d bytes )\n",STOR_DATA_SIZE(&xmlstor));
		__os_free(tmpblk);
		return -1;
	}
	DBG("BLOCK[%d] HEADER DATA READING DONE\n", loc);

	#define _CLOSING_XML_TAG "</config>"

	/* location of final XML closing tag */
	ploc = strcasestr(tmpblk, _CLOSING_XML_TAG );
	sh->block.rsize = ploc - tmpblk + strlen( _CLOSING_XML_TAG ) + 1;
	sh->block.magic = htonl(STORAGE_HEADER_MAGIC);
	sh->block.dirty = 1; /* make it dirty */
	sh->block.crc = _xml_storage_crc( tmpblk, sh->block.rsize );
	sh->block.flags = 0;
	DBG("BLOCK[%d] HEADER - LATEST - REBUILDING DATA %08x size(%d) crc(%x) \n",
	    loc, sh->block.start, sh->block.rsize, sh->block.crc );

	__os_free(tmpblk);
	return 0;
}

/*
 * something like fsck ...
 */
static void _xml_storage_header_print(struct storage *stor, storage_header *blk, char *msg)
{
	int ix;
	storage_header *sh;

	if (msg)
		DBG("[[%s]]\n",msg);

	/* DUMP ALL RECORDs */
	for(ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
		sh = (storage_header *)&(blk[ ix ]);
		DBG("INFO HEAD[ %d ]=0x%08x dirty=%x flags=%x crc=%x size=%x \n",ix,
		    xmlstor.headindex[ ix ], sh->block.dirty, sh->block.flags, sh->block.crc, sh->block.rsize);
	}
	DBG("===========\n");
}

static int _xml_storage_fixup( struct storage *stor )
{
	storage_header *sh;
	storage_header *blk;
	int updated, lb_found;
	int res;

	blk = (storage_header *)__os_malloc( sizeof(storage_header) * STOR_HEADER_NUM(&xmlstor) );
	if (!blk) {
		ERR("__os_malloc(%d x %d) error\n",STOR_HEADER_NUM(&xmlstor),(int)sizeof(storage_header));
		return -1;
	}

	/* STEP. 0 -
	 *
	 * DUMP ALL RECORDs
	 *
	 */
	for( int ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
		if (stor->stor_rewind(stor,
		                      xmlstor.headindex[ ix ] ) < 0) {
			ERR("storage rewind( HEADER [%d] = [0x%08x] ) failed\n", ix, xmlstor.headindex[ ix ]);
			return -1;
		}
		sh = (storage_header *)&(blk[ ix ]);
		if (stor->stor_read(stor,
		                    (char *)sh,
		                    sizeof(storage_header)) != STOR_READ_SUCCESS) {
			ERR("stor_read(HEADER.. %dth)::failed\n",ix);
			continue;
		}
	}
	_xml_storage_header_print(stor, blk, (char *)"DUMP" );

#define _NOTFOUND_ 100

	/* STEP. 1 -
	 *
	 * RECOVER PHYSICALLY BROKEN BLOCK
	 *
	 */
	for( int ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
		sh = (storage_header *)&(blk[ ix ]);
		if ( ntohl(sh->block.magic) == STORAGE_HEADER_MAGIC )
			continue;
		MSG("PURE BROKEN HEADER %d \n",ix);
		if (!xml_storage_data_recover( stor, sh, ix )) {
			sh->block.flags |= STOR_FLAG_RECOVERED;
			sh->block.flags |= STOR_FLAG_TBU;
		} else
			ERR("FIXUP ERROR AT %d\n",ix);
	}
	_xml_storage_header_print(stor, blk, (char *)"BROKEN FIXUP" );

	lb_found = _NOTFOUND_;  /* latest block found */

	/* STEP. 2 -
	 *
	 * SEARCH DIRTY(1)+ONDIRTY(FF) BLOCK AND MAKES IT AS "TBU"
	 *
	 */
	for( int ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
		sh = (storage_header *)&(blk[ ix ]);
		if ( !(sh->block.dirty &&
		       (sh->block.flags & STOR_FLAG_ONDIRTY)) )
			continue;
		MSG("DIRTY+ONDIRTY BLOCK FOUND AT %d \n",ix);
		if (lb_found != _NOTFOUND_) {
			ERR("SYSTEM IS FATALLY CRASHED!\n");
			return -1;
		}
		MSG("    MAKE IT LATEST %d\n", ix);
		sh->block.dirty = 1;
		sh->block.flags &= ~STOR_FLAG_ONDIRTY;
		sh->block.flags |= STOR_FLAG_TBU;
		lb_found = ix;
	}
	_xml_storage_header_print(stor, blk, (char *)"DIRTY+ONDIRTY CHECK" );

	/* STEP. 3 -
	 *
	 * RECOVERED BLOCK CHECK AGAIN
	 *
	 */
	for( int ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
		sh = (storage_header *)&(blk[ ix ]);
		if ( !(sh->block.flags & STOR_FLAG_RECOVERED) )
			continue;
		MSG("STALE BLOCK FOUND AT %d + STALE BLOCK %d\n",ix, lb_found);
		if (lb_found == _NOTFOUND_) {
			sh->block.dirty = 1;
			MSG("   MAKE IT LATEST %d\n", ix);
			lb_found = ix;
		}else{
			sh->block.dirty = 0;
			MSG("   MAKE IT OBSOLETE %d\n", ix);
		}
		sh->block.flags &= ~STOR_FLAG_RECOVERED;
		sh->block.flags |= STOR_FLAG_TBU;
	}
	_xml_storage_header_print(stor, blk, (char *)"RECOVER BLOCK PROCESS" );

	/* STEP. 4 -
	 *
	 * DIRTY BLOCK CHECK AGAIN
	 *
	 */
	if (lb_found != _NOTFOUND_) {
		for( int ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
			sh = (storage_header *)&(blk[ ix ]);
			if ( !sh->block.dirty )
				continue;
			if ( sh->block.flags & STOR_FLAG_TBU )
				continue;

			MSG("DIRTY BLOCK AT %d NOW BECOMES STALE. \n",ix);
			sh->block.dirty = 0;
			sh->block.flags |= STOR_FLAG_TBU;
		}
	}
	_xml_storage_header_print(stor, blk, (char *)"DIRTY BLOCK PROCESS" );

	/* STEP 5 -
	 *
	 * UPDATE BLOCKS
	 */
	updated = 0;
	for( int ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
		sh = (storage_header *)&(blk[ ix ]);
		if ( !(sh->block.flags & STOR_FLAG_TBU) )
			continue;
		MSG("UPDATE BLOCK %d\n",ix);
		sh->block.flags &= ~STOR_FLAG_TBU;
		res = xml_storage_header_fixup_write( stor, ix, (char *)sh );
		MSG("BLOCK[%d] HEADER - FIXUP --> %s \n", ix, res ? "FAILED" : "SUCCESS" );
		updated = 1;
	}

	_xml_storage_header_print(stor, blk, (char *)"FINALLY WRITTEN" );

#ifdef XTEST
	if( updated ) {
		FILE *fp = fopen("/var/tmp/xtest","w+");
		if (fp) {
			fprintf(fp,"%d",0);
			fclose(fp);
		}
	}
	return 0;
#else
	/* System will reboot again under any fixup/update. */
	return updated ? REQ_REBOOT : 0;
#endif
}

int _xml_storage_initialize( char *xmlfn )
{
	struct storage *stor = (struct storage *)xmlstor.stor;
	storage_header *sh;
	struct stat st;
	int fd;
	int t_amount;
	int res;

	/* check file size of xmlfn */
	if (stat(xmlfn,&st) < 0) {
		ERR("%s is not stated\n",xmlfn);
		return -1;
	}

	if ((int)st.st_size >  STOR_DATA_SIZE(&xmlstor)) {
		ERR("%s is too big to put in media\n",xmlfn);
		return -1;
	}

	if (stor->stor_open(stor) != STOR_OPEN_SUCCESS)
		return -1;

	if ( _xml_storage_header_fetch( stor, -1 ) ) {
		stor->stor_close(stor);
		return -1;
	}
	sh = (storage_header *)&xmlstor.header;

	/* DUMP FILE INTO DATA STRUCTURE */
	/* data block memory space */
	xmlstor.data = (char *)__os_malloc( STOR_DATA_SIZE(&xmlstor) );
	if (!xmlstor.data) {
		ERR("__os_malloc(%d) error\n",STOR_DATA_SIZE(&xmlstor));
		stor->stor_close(stor);
		return -1;
	}

	/*
	 * xmlfn -> xmlfn of XML file maintained in generic file system space.
	 * We can use UNIX open/read/write/close to manipulate this file.
	 *
	 */
	DBG("INITIAL DATA COPY FROM %s \n", xmlfn);

	/* dump all */
	fd = open(xmlfn,O_RDONLY);
	if (!fd) {
		ERR("%s cannot be opened\n",xmlfn);
		__os_free( xmlstor.data );
		stor->stor_close( stor );
		return -1;
	}

	t_amount = 0;
	while( t_amount < (int)st.st_size ) {
		res = read(fd,
		           xmlstor.data + t_amount,
		           (int)st.st_size - t_amount );
		DBG("%s read %d/%d \n",xmlfn,res,(int)st.st_size);
		if (res < 0) {
			ERR("%s reading error at %d/%d \n",xmlfn,t_amount,(int)st.st_size);
			close(fd);
			break;
		}
		t_amount += res;
	}
	close(fd);

	/* update header */
	sh->block.rsize = (int)st.st_size;
	sh->block.crc = _xml_storage_crc( xmlstor.data, sh->block.rsize );
	DBG("SIZE=%d CRC=0x%04x DATA=%08x \n", sh->block.rsize, sh->block.crc, sh->block.start );

	/* header writing */
	if( stor->stor_rewind( stor,
	                       xmlstor.headindex[ xmlstor.hindex ] ) < 0 ) {
		ERR("storage rewind( HEADER [%d] = [0x%08x] ) failed\n",
		    xmlstor.hindex, xmlstor.headindex[ xmlstor.hindex ] );
		__os_free( xmlstor.data );
		stor->stor_close(stor);
		return 1;
	}
	if( stor->stor_write( stor, (char *)&(xmlstor.header),
	                      STORAGE_HEADER_SIZE ) != STOR_WRITE_SUCCESS ) {
		ERR("Header writing to media::error\n");
		__os_free( xmlstor.data );
		stor->stor_close(stor);
		return 1;
	}

	/* data writing */
	if( stor->stor_rewind( stor,
	                       sh->block.start ) < 0 ) {
		ERR("storage rewind( DATA [0x%08x] ) failed\n", sh->block.start);
		__os_free( xmlstor.data );
		stor->stor_close(stor);
		return 1;
	}
	if( stor->stor_write( stor, (char *)xmlstor.data,
	                      STOR_DATA_SIZE(&xmlstor) ) != STOR_WRITE_SUCCESS ) {
		ERR("Header writing to media::error\n");
		__os_free( xmlstor.data );
		stor->stor_close(stor);
		return 1;
	}

	__os_free( xmlstor.data );
	stor->stor_close(stor);

	return 0;
}

/*
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *
 * P U B L I C                   I N T E R F A C E
 *
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 */
#define STOR_API_NOTEXIST   \
	(!stor->stor_select ||  \
	 !stor->stor_open ||    \
	 !stor->stor_write ||   \
	 !stor->stor_read ||    \
	 !stor->stor_rewind ||  \
	 !stor->stor_deselect ||    \
	 !stor->stor_close )

void xml_storage_select( int type )
{
	struct storage *stor = xmlstor.stor;

	if ( stor ) {
		ERR("XML STORAGE ALREADY SELECTED\n");
		return;
	}

	xmlstor.stor = (struct storage *)&(stor_interface[ type ]);
}

void xml_storage_deselect( void )
{
	struct storage *stor = xmlstor.stor;

	if ( !stor )
		return;

	if ( STOR_API_NOTEXIST )
		return;

	stor->stor_deselect( stor );

	xmlstor.stor = NULL;
}

int xml_storage_open( char *dfn, char *xmlfn, storage_info *sinfo )
{
	struct storage *stor = (struct storage *)xmlstor.stor;
	int ix, res;
	int scount;
	int hcount;
	storage_header *sh;
	static char pbuf[1024];
	int pindex = 0;


	if (!stor || !dfn || !sinfo)
		return 1;

	if (stor->info) {
		ERR("file storage information is allocated!!\n");
		return 1;
	}

	if (STOR_API_NOTEXIST)
		return 1;

	if ((stor->info = stor->stor_select(dfn)) == NULL) {
		ERR("storage select error\n");
		return 1;
	}

	/* fill out physical data */
	xmlstor.hnum = sinfo->hnum;
	xmlstor.bnum = sinfo->bnum;
	xmlstor.bsize= sinfo->bsize << 10;
	xmlstor.dmin = ((xmlstor.bsize) >= DEFAULT_STORAGE_DATA_SIZE) ?
	               (STOR_DATA_SIZE(&xmlstor) / STORAGE_IO_SIZE) >> 1 : 1;

	MSG("%s Physical Information = ( %d x %d Kbytes + %d x %d Kbytes ) %d \n",
	    dfn,
	    STOR_HEADER_NUM(&xmlstor),
	    STORAGE_HEADER_SIZE >> 10,
	    STOR_DATA_NUM(&xmlstor),
	    STOR_DATA_SIZE(&xmlstor) >> 10,
	    STOR_DATA_MIN(&xmlstor) );

	/* look at file */
	res = stor->stor_open(stor);
	switch( res ) {
	case STOR_OPEN_CREATED:
	{
		char __dummy[ STORAGE_IO_SIZE ]; /* small data */
		DBG("%s has been created\n",dfn);

		memset(__dummy,0,sizeof(__dummy));
		for (ix = 0; ix < (int)(STOR_SIZE(&xmlstor)/(int)sizeof(__dummy)); ix++) {
			if( stor->stor_write(stor, __dummy, sizeof(__dummy))
			    != STOR_WRITE_SUCCESS ) {
				ERR("%s zeroing error at %d\n",dfn,ix);
				goto __exit_xml_storage_open;
			}
		}
		DBG("%s zeored \n",dfn);
	}
	break;
	case STOR_OPEN_FAILURE:
		DBG("%s is not accessible\n",dfn);
		break;
	case STOR_OPEN_SUCCESS:
		DBG("%s has been opened\n",dfn);
		break;
	}
	stor->stor_close(stor);

	if (res == STOR_OPEN_FAILURE)
		goto __exit_xml_storage_open;

	xmlstor.size = stor->stor_size(stor); /* actual file size */
	MSG("FILE %s SIZE=%d \n", dfn, xmlstor.size );

	sh = (storage_header *)&xmlstor.header;

	/* header check */
	if ((res = stor->stor_open(stor)) != STOR_OPEN_SUCCESS) {
		ERR("%s storage open error for header check\n", dfn);
		goto __exit_xml_storage_open;
	}

	/* INDEX CREATION */

	/* HEADER */
	xmlstor.headindex = (unsigned int *)__os_malloc(
		sizeof(unsigned int) * STOR_HEADER_NUM(&xmlstor) );
	if( !xmlstor.headindex ) {
		ERR("%s headindex malloc error\n", dfn);
		goto __exit_xml_storage_open;
	}

	/* DATA */
	xmlstor.dataindex = (unsigned int *)__os_malloc(
		sizeof(unsigned int) * STOR_DATA_NUM(&xmlstor) );
	if( !xmlstor.dataindex ) {
		ERR("%s dataindex malloc error\n", dfn);
		goto __exit_xml_storage_open;
	}

	/* HEADER SPACE SCANNING */
	scount = 0; /* success count */
	hcount = 0; /* header check count */
	for( ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ ) {
		res = stor->stor_read(stor,
		                      (char *)sh,
		                      sizeof(storage_header) );
		if (res != STOR_READ_SUCCESS) {
			ERR("%s header analysis error \n", dfn);
			goto __exit_xml_storage_open;
		}
		xmlstor.headindex[ ix ] =
			stor->stor_rewind(stor,-1) - sizeof(storage_header);
		++scount;
		if ( ntohl(sh->block.magic) == STORAGE_HEADER_MAGIC ) {
			++hcount;
			DBG("HEADER[ %d ] FLAGS=%x DIRTY=%d \n", ix, sh->block.flags, sh->block.dirty);
		}
	}
	xmlstor.dataindex[ 0 ] =
		(xmlstor.headindex[ ix-1 ] + STORAGE_HEADER_SIZE);

	MSG("%s : HEADER ANALSYS [%d/%d] \n", __func__, hcount, scount);
	{
		pindex = 0;
		memset(pbuf,0,1024);
		pindex += sprintf(pbuf+pindex,"%s : HEADER LOCATION = ( ",__func__);
		for( ix = 0; ix < STOR_HEADER_NUM(&xmlstor); ix++ )
			pindex += sprintf(pbuf+pindex,"%08x ",xmlstor.headindex[ ix ]);
		pindex = sprintf(pbuf+pindex,")");
		MSG("%s\n",pbuf);
	}
	for( ix = 1; ix < STOR_DATA_NUM(&xmlstor); ix++ )
		xmlstor.dataindex[ ix ] =
			xmlstor.dataindex[ ix - 1 ] + STOR_DATA_SIZE(&xmlstor);

	/* CHECK DATA SPACE */
	{
		int ix, iy;
		int count;
		int start, nloc;
		static char tmpblk[ STORAGE_IO_SIZE ];

		for( ix = 0; ix < STOR_DATA_NUM(&xmlstor); ix++ ) {
			start = xmlstor.dataindex[ ix ];
			if ( stor->stor_rewind( stor, start ) != start ) {
				ERR("storage rewind( DATA [0x%08x] ) failed\n", start);
				goto __exit_data_check;
			}
			count = 0;
			for( iy = 0; iy < (STOR_DATA_SIZE(&xmlstor) / STORAGE_IO_SIZE); iy++ ) {
				stor->stor_read(stor, (char *)tmpblk, STORAGE_IO_SIZE);
				nloc = stor->stor_rewind(stor,-1);
				if (nloc == (start + STORAGE_IO_SIZE))
					count++;
				start = nloc;
			} /* for( iy ... */
			if (count < STOR_DATA_MIN(&xmlstor)) {
				ERR("DATA BLOCK [ %d ] JUST HAVE %d/%d BLOCKS. \n", ix, count, STOR_DATA_MIN(&xmlstor) );
				goto __exit_xml_storage_open;
			}
			MSG("DATA[ %d ] has %d blocks \n",ix,count);
		} /* for( ix ... */
__exit_data_check:
		do {} while(0);
	} /* if ( 1 ) */

	{
		pindex = 0;
		memset(pbuf,0,1024);
		pindex += sprintf(pbuf+pindex,"%s : DATA LOCATION = ( ",__func__);
		for( ix = 0; ix < STOR_DATA_NUM(&xmlstor); ix++ )
			pindex += sprintf(pbuf+pindex,"%08x ",xmlstor.dataindex[ ix ]);
		pindex = sprintf(pbuf+pindex,")");
		MSG("%s\n",pbuf);
	}

	stor->stor_close(stor);

	/* analysis current situation */
	if (scount > 0) {
		if (hcount == 0) {
			/* There is header blocks which we can read successfully
			 * but all of them were not initialized */
			DBG("%s first boot up \n", dfn);
			if ( _xml_storage_clean() ) {
				ERR("%s clean up failed\n",dfn);
				goto __exit_xml_storage_open;
			}
			if ( _xml_storage_initialize( xmlfn ) ) {
				ERR("%s initial failed\n",xmlfn);
				goto __exit_xml_storage_open;
			}
		} else {
			/* There is header blocks which we can read successfully
			 * and also some of them has been configured properly */
			DBG("%s configured and prepared before \n", dfn);
			/* fix up */
			stor->stor_open(stor);
			res = _xml_storage_fixup( stor );
			stor->stor_close(stor);

			if (res == REQ_REBOOT) {
				DBG("INVOKING SYSTEM REBOOT....\n");
				system( "/etc/init.d/reboot" );
			} else
			if (res) {
				DBG("SYSTEM FIXUP FAILED..... \n");
				goto __exit_xml_storage_open;
			}
		}
	} else {
		DBG("%s system total failure \n", dfn);
		goto __exit_xml_storage_open;
	}

	return 0;

__exit_xml_storage_open:
	stor->stor_deselect(stor);
	return 1;
}

char * xml_storage_validate( char *xmlfn )
{
	struct storage *stor = xmlstor.stor;
	storage_header *sh;
	char *txmlfn = NULL;
	char *p = NULL;
	int fd;
	int t_amount;
	int res;
	unsigned short calc_crc;

	if (!stor)
		return NULL;

	if (!stor->info)
		return NULL;

	if (stor->stor_open(stor) != STOR_OPEN_SUCCESS)
		return NULL;

	/* storage header */
	if ( _xml_storage_header_fetch( stor, -1 ) ) {
		stor->stor_close(stor);
		return NULL;
	}
	sh = (storage_header *)&xmlstor.header;

	MSG("HEADER[%-2d]::MAGIC=%s \n",xmlstor.hindex,(ntohl(sh->block.magic) == STORAGE_HEADER_MAGIC) ? "OK" : "NOT OK");
	MSG("HEADER[%-2d]::RSIZE=%d \n",xmlstor.hindex,sh->block.rsize);
	MSG("HEADER[%-2d]::START=%x \n",xmlstor.hindex,sh->block.start);
	MSG("HEADER[%-2d]::CRC  =%x \n",xmlstor.hindex,sh->block.crc);

	/* data block memory space */
	xmlstor.data = (char *)__os_malloc( STOR_DATA_SIZE(&xmlstor) );
	if (!xmlstor.data) {
		ERR("__os_malloc(%d) error\n",STOR_DATA_SIZE(&xmlstor));
		stor->stor_close(stor);
		return NULL;
	}

	/* go to the position of data block */
	if ( stor->stor_rewind( stor,
	                        (int)sh->block.start )  != (int)sh->block.start ) {
		ERR("storage rewind( DATA [0x%08x] ) failed\n", sh->block.start);
		goto __exit_xml_storage_validate;
	}

	/* read data block */
	if ( stor->stor_read( stor,
	                      xmlstor.data,
	                      STOR_DATA_SIZE(&xmlstor) ) != STOR_READ_SUCCESS ) {
		ERR("stor_read( .. %d bytes )\n",STOR_DATA_SIZE(&xmlstor));
		goto __exit_xml_storage_validate;
	}

	stor->stor_close(stor);

	/* CHECK CRC */
	if ( (calc_crc = _xml_storage_crc( xmlstor.data,
	                                   sh->block.rsize )) != sh->block.crc ) {
		ERR("CRC BROKEN !! (%04x != %04x) \n", sh->block.crc, calc_crc );
		goto __exit_xml_storage_validate;
	}

	/* eg.) "/etc/config.xml" --> "/var/tmp/config.xml" */
	txmlfn = (char *)__os_malloc( PATH_MAX + 1 );
	if (!txmlfn) {
		ERR("malloc() - failed\n");
		goto __exit_xml_storage_validate;
	}
	memset( txmlfn, 0, PATH_MAX+1 );
	/*strcpy( txmlfn , TEMP_XML_PREFIX ); */
	sprintf( txmlfn, TEMP_XML_PREFIX "/%d",getpid());
	if ((p = strrchr( xmlfn, '/' )) != NULL) {
		strcat( txmlfn, p );
	}else {
		strcat( txmlfn, "/" );
		strcat( txmlfn, xmlfn );
	}

	MSG("Temporary XML file name : (%s) \n",txmlfn);

	unlink( txmlfn ); /* delete if exist */
	fd = open( txmlfn, O_RDWR | O_CREAT, 0644 );
	if (fd < 0) {
		ERR("open( %s )::failed\n", txmlfn );
		goto __exit_xml_storage_validate;
	}

	t_amount = 0;
	while( t_amount < (int)sh->block.rsize ) {
		res = write(fd,
		            xmlstor.data + t_amount,
		            (int)sh->block.rsize - t_amount );
		DBG("%s write %d/%d \n",xmlfn,res,(int)sh->block.rsize);
		if (res < 0) {
			ERR("%s writing error at %d/%d \n",xmlfn,t_amount,(int)sh->block.rsize);
			close(fd);
			goto __exit_xml_storage_validate;
		}
		t_amount += res;
	}

	close(fd);
	stor->stor_close(stor);

	MSG("STORAGE VALIDATED [%d] \n",xmlstor.hindex);

	return txmlfn;

__exit_xml_storage_validate:
	if (xmlstor.data)
		__os_free( xmlstor.data );
	if (txmlfn)
		__os_free( txmlfn );
	stor->stor_close(stor);
	stor->stor_deselect(stor);
	return NULL;
}

/* run time function */
int xml_storage_update( char * dfn, char *xmlfn )
{
	struct storage *stor = xmlstor.stor;
	storage_header *sh;
	int fd = 0;
	int t_amount;
	int res;
	int nblock, cblock;

	if (!stor)
		return 1;

	if (!stor->info) {
		if ((stor->info = stor->stor_select(dfn)) == NULL) {
			ERR("storage select error\n");
			return 1;
		}
	}

#ifdef XTEST
	{
		FILE *fp = fopen("/var/tmp/xtest","r+");
		if (fp) {
			fscanf(fp,"%d",&test_method);
			fclose(fp);
		}
	}
#endif

	/*
	 * STEP 1 : xmlstor.hindex  <-- current location
	 *           && nblock   <-- next location
	 */

	if (stor->stor_open(stor) != STOR_OPEN_SUCCESS)
		return 1;

	/* storage header */
	if ( _xml_storage_header_fetch( stor, -1 ) ) {
		stor->stor_close(stor);
		return 1;
	}

	/* CURRENT BLOCK */
	cblock = xmlstor.hindex;

	/* switching to next block */
	nblock = cblock + 1;

	while( nblock != cblock) {
		if (nblock >= STOR_HEADER_NUM(&xmlstor))
			nblock = 0;
		if ( !_xml_storage_header_fetch( stor, nblock ) )
			break;
		nblock = nblock + 1;
	}

	MSG("STORAGE SWITCH FROM %d TO %d \n", cblock, nblock);

	stor->stor_close(stor);

	/*
	 * STEP 2 : Write to new block
	 *
	 */

	if (stor->stor_open(stor) != STOR_OPEN_SUCCESS)
		return 1;

	/* storage header */
	if ( _xml_storage_header_fetch( stor, nblock ) ) {
		stor->stor_close(stor);
		return 1;
	}

	/* Make dirty new block ... */

	sh = (storage_header *)&xmlstor.header;

	/* data block memory space */
	xmlstor.data = (char *)__os_malloc( STOR_DATA_SIZE(&xmlstor) );
	if (!xmlstor.data) {
		ERR("__os_malloc(%d) error\n",STOR_DATA_SIZE(&xmlstor));
		stor->stor_close(stor);
		return 1;
	}

	/* go to the position of data block */
	if ( stor->stor_rewind( stor,
	                        (int)sh->block.start )  != (int)sh->block.start ) {
		ERR("storage rewind( DATA [0x%08x] ) failed\n", sh->block.start);
		goto __exit_xml_storage_update;
	}

	fd = open( xmlfn, O_RDWR, 0644 );
	if (fd < 0) {
		ERR("open( %s )::failed\n", xmlfn );
		goto __exit_xml_storage_update;
	}

	/*
	 * STEP.1 -- DATA BLOCK UPDATE  ----
	 */

	/* dump file --> block */
	t_amount = 0;
	while( 1 ) {
		/* read file */
		res = read(fd,
		           xmlstor.data + t_amount,
		           (int)STORAGE_IO_SIZE );
		DBG("%s read %d/%d \n",xmlfn,res,(int)STORAGE_IO_SIZE);
		if (res <= 0)
			break;
		t_amount += res;
	}
	close(fd);
	fd = 0;

	sh->block.rsize = (int)t_amount; /* TREE SIZE */
	DBG("XML TREE (%s) SIZE = %d[%d]\n",xmlfn, (int)t_amount, (int)sh->block.rsize);
	//DBG("XML TREE = \n(%s)\n", xmlstor.data);

	/* copy block --> media file */
	t_amount = 0;
	while( t_amount < (int)STOR_DATA_SIZE(&xmlstor) ) {
		res = stor->stor_write( stor,
		                        xmlstor.data + t_amount,
		                        STORAGE_IO_SIZE );
		/* write to storage */
		if ( res != STOR_WRITE_SUCCESS ) {
			ERR("stor_read( .. %d bytes )\n",res);
			goto __exit_xml_storage_update;
		}
		t_amount += STORAGE_IO_SIZE;
	}

	/* update CRC */
	sh->block.crc = _xml_storage_crc( xmlstor.data, sh->block.rsize );

	/* update dirtiness */
	sh->block.dirty = 1;

	/* very important */
	sh->block.flags = STOR_FLAG_ONDIRTY;

#if 0
	{
		struct XmlData * data = kxml_parse( kxml_header( xmlstor.data ), 0 );
		if (data) {
			kxml_print(data);
			kxml_free(data);
		}
	}
#endif

	__os_free( xmlstor.data );
	xmlstor.data = NULL;

	/*
	 * STEP.2 -- HEADER BLOCK UPDATE ---
	 */

	DBG("HEADER[%-2d]::MAGIC=%s \n",nblock,(ntohl(sh->block.magic) == STORAGE_HEADER_MAGIC) ? "OK" : "NOT OK");
	DBG("HEADER[%-2d]::RSIZE=%d \n",nblock,sh->block.rsize);
	DBG("HEADER[%-2d]::START=%8x\n",nblock,sh->block.start);
	DBG("HEADER[%-2d]::CRC  =%4x\n",nblock,sh->block.crc);

	/* header writing */
	if ( stor->stor_rewind( stor,
	                        xmlstor.headindex[ nblock ] )
	     != (int)(xmlstor.headindex[ nblock ]) ) {
		ERR("storage rewind( HEADER [%d] = [0x%08x] ) failed\n", nblock, xmlstor.headindex[ nblock ]);
		goto __exit_xml_storage_update;
	}

	if( stor->stor_write( stor,
	                      (char *)&(xmlstor.header),
	                      STORAGE_HEADER_SIZE )
	    != STOR_WRITE_SUCCESS ) {
		ERR("Header writing to media::error\n");
		goto __exit_xml_storage_update;
	}

	stor->stor_close(stor);

	MSG("DIRTY NEW BLOCK [ %d ] = %08x FLAGS=%x DIRTY=%d \n", nblock, xmlstor.headindex[ nblock ], sh->block.flags, sh->block.dirty );

#ifdef XTEST
	if (test_method == 2) {
		DBG("XTEST :: EXIT AFTER WRITE NEW BLOCK \n");
		exit (1);
	}
	if (test_method == 8) {
		char xx[STORAGE_HEADER_SIZE];
		DBG("XTEST :: EXIT AFTER ERASE NEW BLOCK \n");
		memset( xx, 0xFF, STORAGE_HEADER_SIZE );
		stor->stor_open(stor);
		stor->stor_rewind( stor, xmlstor.headindex[ nblock ] );
		stor->stor_write( stor, (char *)&xx, STORAGE_HEADER_SIZE );
		stor->stor_close(stor);
		exit (1);
	}
#endif

	/*
	 *
	 * STEP 3. Undirty current block
	 *
	 */
	if (stor->stor_open(stor) != STOR_OPEN_SUCCESS)
		return 1;

	/* storage header */
	if ( _xml_storage_header_fetch( stor, cblock ) ) {
		stor->stor_close(stor);
		return 1;
	}
	sh = (storage_header *)&xmlstor.header;

	/* UNDIRTY CURRENT BLOCK */
	sh->block.dirty = 0;

	/* very important */
	sh->block.flags = STOR_FLAG_CLEAN;

	/* header writing */
	if ( stor->stor_rewind( stor,
	                        xmlstor.headindex[ cblock ] )
	     != (int)(xmlstor.headindex[ cblock ]) ) {
		ERR("storage rewind( HEADER [%d] = [0x%08x] ) failed\n", cblock, xmlstor.headindex[ cblock ]);
		goto __exit_xml_storage_update;
	}

	if( stor->stor_write( stor,
	                      (char *)&(xmlstor.header), STORAGE_HEADER_SIZE )
	    != STOR_WRITE_SUCCESS ) {
		ERR("Header writing to media::error\n");
		goto __exit_xml_storage_update;
	}

	stor->stor_close(stor);

	MSG("UNDIRTY CURRENT BLOCK [ %d ] = %08x FLAGS=%x DIRTY=%d \n", cblock, xmlstor.headindex[ cblock ], sh->block.flags, sh->block.dirty );

#ifdef XTEST
	if (test_method == 3) {
		DBG("XTEST :: EXIT AFTER UNDIRTY CURRENT BLOCK \n");
		exit (1);
	}
	if (test_method == 9) {
		char xx[STORAGE_HEADER_SIZE];
		DBG("XTEST :: EXIT AFTER ERASE UNDIRTY CURRENT BLOCK \n");
		memset( xx, 0xFF, STORAGE_HEADER_SIZE );
		stor->stor_open(stor);
		stor->stor_rewind( stor, xmlstor.headindex[ cblock ] );
		stor->stor_write( stor, (char *)&xx, STORAGE_HEADER_SIZE );
		stor->stor_close(stor);
		exit (1);
	}
#endif

	/*
	 *
	 * STEP 4. CLEAN UP ONDIRTY FLAG
	 *
	 */
	if (stor->stor_open(stor) != STOR_OPEN_SUCCESS)
		return 1;

	/* storage header */
	if ( _xml_storage_header_fetch( stor, nblock ) ) {
		stor->stor_close(stor);
		return 1;
	}
	sh = (storage_header *)&xmlstor.header;

	if (sh->block.flags != STOR_FLAG_ONDIRTY)
		ERR("DATA [%d] BLOCK INCONSISTENT !!\n", nblock);

	/* very important */
	sh->block.flags = STOR_FLAG_CLEAN;

	/* header writing */
	if ( stor->stor_rewind( stor,
	                        xmlstor.headindex[ nblock ] )
	     != (int)(xmlstor.headindex[ nblock ]) ) {
		ERR("storage rewind( HEADER [%d] = [0x%08x] ) failed\n", nblock, xmlstor.headindex[ nblock ]);
		goto __exit_xml_storage_update;
	}

	if( stor->stor_write( stor,
	                      (char *)&(xmlstor.header), STORAGE_HEADER_SIZE )
	    != STOR_WRITE_SUCCESS ) {
		ERR("Header writing to media::error\n");
		goto __exit_xml_storage_update;
	}

	stor->stor_close(stor);

	MSG("CLEAN UP ONDIRTY [ %d ] = %08x FLAGS=%x DIRTY=%d \n", nblock, xmlstor.headindex[ nblock ], sh->block.flags, sh->block.dirty);

#ifdef XTEST
	if (test_method == 4) {
		DBG("XTEST :: EXIT AFTER CLEAN UP NEW BLOCK \n");
		exit (1);
	}
	if (test_method == 10) {
		char xx[STORAGE_HEADER_SIZE];
		DBG("XTEST :: EXIT AFTER ERASE CLEAN UP NEW BLOCK \n");
		memset( xx, 0xFF, STORAGE_HEADER_SIZE );
		stor->stor_open(stor);
		stor->stor_rewind( stor, xmlstor.headindex[ nblock ] );
		stor->stor_write( stor, (char *)&xx, STORAGE_HEADER_SIZE );
		stor->stor_close(stor);
		exit (1);
	}
#endif

	stor->stor_deselect(stor);

	/* MSG("STORAGE UPDATE \n"); */
	printf("STORAGE UPDATE \n");

	return 0;

__exit_xml_storage_update:
	if (fd)
		close(fd);
	if (xmlstor.data)
		__os_free( xmlstor.data );
	stor->stor_close(stor);
	stor->stor_deselect(stor);
	return -1;
}

