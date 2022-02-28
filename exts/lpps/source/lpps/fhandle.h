#ifndef __FILE_HANDLE_HEADERS__
#define __FILE_HANDLE_HEADERS__

//
//
// Actually, one of the difficult controls inside of file system is the file handle management. 
// Since it is strictly related to "limitation of outstanding files" and usually "vector"/"array" - like
// data structure is used and sometimes (ext4) manages an hierarchy. 
//
//

extern int prepare_fh( int max_files );
extern int get_fh( void * );
extern void * get_priv( int );
extern int put_fh( int );

#endif /* __FILE_HANDLE_HEADERS__ */
