#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "fhandle.h"
#include "os.h"
#include "malloc.h"

static unsigned char * fh_vector; // handle index vector 
static long * fh_priv_vector;     // private structure vector 
static int fh_vector_len; 	  // vector byte length 
static int fh_max_files;	  // max files

#define BYTE_SHIFT 3 /* 2^3 = 8 */

#define __encode( nth, bit ) 	(((nth)<< BYTE_SHIFT)|((bit)&0x7))
#define __decode_nth( fh )	((fh) >> BYTE_SHIFT)
#define __decode_bit( fh )	((fh)&(0x7))

#define valid_fh(code)		(((code) >= 0) && ((code) < fh_max_files))

//
//
//
// P U B L I C       F U N C T I O N S 
//
//
//
//
int prepare_fh( int max_files )
{
	fh_vector_len = max_files / (sizeof(unsigned char) << BYTE_SHIFT);

	// file handle vector 
	fh_vector = (unsigned char *)os_malloc( fh_vector_len );
	if (!fh_vector) {
		return -1;
	}
	memset( fh_vector, 0, fh_vector_len );

	// private structure 
	fh_priv_vector = (long *)os_malloc( max_files * sizeof(long) );
	if (!fh_priv_vector) {
		os_free( fh_vector );
		return -1;
	}
	memset( fh_priv_vector, 0, max_files * sizeof(long) );

	// global variable - max number of files ...
	fh_max_files = max_files;

	return 0;
}

int get_fh( void *priv )
{
	int i,j;
	int new_fh;
	unsigned char val;
	unsigned char inuse;

	for (i = 0; i < fh_vector_len; i++) {
		val = fh_vector[ i ];
		// bit mask ... 
		for (j = 0; j < 8; j++) {
			inuse = (val & (0x80 >> j));
			if (inuse)
		       		continue;
			new_fh = __encode( i, j );
			if (!valid_fh(new_fh)) {
				ERR(" file handle error (%08x) \n",new_fh);
				return -1;
			}
			fh_vector[ i ] |= (0x80 >> j) ;
			fh_priv_vector[ new_fh ] = (long)priv; // private vector 

			return new_fh;
		}
	}

	ERR(" FATAL!! NO FILE HANDLE \n");
	return -1;
}

void * get_priv( int fh )
{
	if (!valid_fh(fh)) {
		ERR(" FATAL!! NO VALID FILE HANDLE \n");
		return NULL;
	}

	return (void *)fh_priv_vector[ fh ];
}

int put_fh( int fh )
{
	int nth;
	int bit;
	unsigned char inuse;

	if (!valid_fh(fh)) {
		ERR(" FATAL!! NO VALID FILE HANDLE \n");
		return -1;
	}

	nth = __decode_nth( fh );
	bit = __decode_bit( fh );
	inuse = fh_vector[ nth ] & (0x80 >> bit);
	if (!inuse) {
		ERR(" FATAL!! INSANITY FILE HANDLE \n");
		return -1;
	}

	// clear bit... 
	fh_vector[ nth ] &= ~(0x80 >> bit);

	fh_priv_vector[ fh ] = (long)NULL; // clearing 

	return 0;
}

//
// We don't have destructor function for file handle container...
//
