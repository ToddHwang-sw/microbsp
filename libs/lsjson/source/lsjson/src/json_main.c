#include "bcm.h"

extern u32 json_dbg;  /* json debug flag */
extern int mem_dbg; /* memory history */

int sys_rmem = 4*1024*1024;

#define MAXTREE 16
typedef struct {
	int used;
	sjson_table_t tbl;
} jtbl_info_t;
static jtbl_info_t info[ MAXTREE ] = { {0,}, };

/* empty slot */
static int jtbl_find_avail( void )
{
	int i;

	for (i = 0; i < MAXTREE; i++) {
		if (info[ i ].used) 
			continue;
		return i;
	}
	return -1;
}

static jtbl_info_t * jtbl_alloc( int index )
{
	if (!info[ index ].used) {
		info[ index ].used = 1;
		return &(info[ index ]);
	} else
		return NULL;
}

static int jtbl_free( int index )
{
	if (!info[ index ].used)
		return -1;

	info[ index ].used = 0;
	return 0;
}

extern void dump_mem_block( void * blk );

/* input command restriction */
#define INPUTLEN 1024
#define MAXARGS 16

/* parse command line */
static int parse_cmd( char *cmds, char *args[] , int *nth )
{
	int ii;
	int index, count;

	/* parse... */
	index = 0;
	count = strlen(cmds);

	*nth = 0;

	for (ii = 0; ii < MAXARGS; ii++)
		args[ ii ] = NULL;

	while (index < count) {
		/* skipping head blank... */
		while( (cmds[index] == ' ') && (index < count) ) 
			++index;
		if ( index >= count ) 
			return -1;
		args[ (*nth)++ ] = (char *)&(cmds[index]);
		while(( (cmds[index] != ' ') &&
				(cmds[index] != '\n')) && (index < count) ) 
			++index;
		cmds[index++] = 0x0;
		if ( (*nth) >= MAXARGS)
			return -1;
	}
	return ((*nth) == 0 ) ? -1: 0;
}

static int process_commands( char *argv[] , int nth )
{
	char * eptr;
	int res;
	/*int done; */
	int index;
	jtbl_info_t *tbl;	
	#define DSIZE 8192
	s8 xbuff[ DSIZE ];

	do {
		if (!strcasecmp(argv[0],"HELP")) {
			printf("\tLOAD   <json file name >\n");
			printf("\tPRINT  <INDEX>  [XML/FILE] [FILE NAME] \n");
			printf("\tPATH   <INDEX>  \n");
			printf("\tFIND   <INDEX>  <PATH>  [PARTIAL] \n");
			printf("\tDELETE <INDEX>  [PATH]  \n");
			printf("\tADD    <INDEX>  <INDEX> \n");
			printf("\tTREE   <INDEX>  [AMOUNT]\n");
			printf("\tRUN    <file> \n");
			printf("\tDBG    \n");
			printf("\tMDBG   \n");
			printf("\tSTATUS \n");
		} else 
		if (!strcasecmp(argv[0],"LOAD")) {
			struct stat st;
			u32 fsize;
			u8 *space;
			int fd;
			int tot_amount;
			int amount;
			jtbl_info_t *tbl;

			if (nth < 2)
				return 0;

			tbl = NULL;

			/* loading into file ... */
			if (lstat(argv[1],&st) == -1) {
				os_err("Error .. no file (%s)\n",argv[1]);
				return 0;
			}

			/* allocating memory */
			fsize = st.st_size; /* file size */
			space = os_malloc( fsize+4 );
			if (!space) {
				os_err("Error os_malloc( %d ) \n", fsize);
				return 0;
			}
			memset(space,0,fsize+4);
			fd = open(argv[1],O_RDONLY);
			if (fd < 0) {
				os_err("Open error\n");
				os_free( space );
				return 0;
			}
	
			/* file reading.. */
			tot_amount = 0;
			do {
				amount = read( fd, space+tot_amount, fsize-tot_amount ); /* reading up total file... */
				if (amount < 0) {
					os_err("read( .. %d )::failed\n", tot_amount);
					os_free(space);
					close(fd);
					return 0;
				}
				tot_amount += amount; 
			} while( tot_amount < fsize );

			close(fd);

			/* preparing for a table */
			if ((index = jtbl_find_avail()) == (-1))
				return 0;
			tbl = jtbl_alloc( index );

			res = sjson_parse( &(tbl->tbl), space );

			os_free(space);

			printf("%s",res?"Failed":"Succeeded");
			if (!res) printf(" %d",index);
			printf("\n");
		} else 
		if (!strcasecmp(argv[0],"FIND")) {
			char * path;
			int nindex;
			jtbl_info_t *ntbl;
			sjson_table_t *t;
			sjson_node_t *ntree;
			int mode;

			if (nth < 3)
				return 0;

			path =  NULL;
			ntbl =  NULL;
			ntree = NULL;

			index = strtol( argv[1] , &eptr, 10 );
			if (!(index >= 0 && index < MAXTREE) ||
					(eptr == argv[1]))
				return 0;
			tbl = &(info[ index ]);
			if (!tbl->used) 
				return 0;
		
			path = argv[2];	
	
			mode = 0;
			if (nth >= 4) {
				if (!strcasecmp(argv[3],"PARTIAL"))
					mode = 1;
			}

			memset(xbuff, 0, DSIZE);
		
			t = &(tbl->tbl);

			ntree = sjson_find_json( t->root[0] , path , mode );
			if (!ntree)
				return 0;
			
			/* preparing for a new table */
			if ((nindex = jtbl_find_avail()) == (-1))
				return 0;
			ntbl = jtbl_alloc( nindex );
			sjson_table_init( &(ntbl->tbl) );
			ntbl->tbl.root[0] = ntree;

			printf("Succeeded %d\n",nindex);
		} else	
		if (!strcasecmp(argv[0],"PRINT") ||
				!strcasecmp(argv[0],"PATH")) {
			char * fmt;
			char * outfn;

			fmt = NULL;
			outfn = NULL;

			/* printing a selected tree in JSON/XML format */
			/* deleting the selected tree */
			if (nth < 2)
				return 0;

			index = strtol( argv[1] , &eptr, 10 );
			if (!(index >= 0 && index < MAXTREE) ||
					(eptr == argv[1]))
				return 0;

			tbl = &(info[ index ]);
			if (!tbl->used) 
				return 0;
		
			if (nth >= 3)
				fmt = argv[2];	

			if (nth >= 4)
				outfn = argv[3];

			memset(xbuff, 0, DSIZE);
			if (!strcasecmp(argv[0],"PRINT")) {
				if (fmt) {
					if (!strcasecmp(fmt,"XML"))
						res = sjson_print_xml( &tbl->tbl , xbuff, DSIZE );
					else
						res = sjson_print_json( &tbl->tbl , xbuff, DSIZE );
				} else {
					res = sjson_print_json( &tbl->tbl , xbuff, DSIZE );
				}
				if (outfn) {
					int fd;

					/* file writing !! */
					fd = open(outfn,O_RDWR|O_CREAT,0666);
					if (fd > 0) {
						write(fd, xbuff, res);
						close(fd);
					}
				} else
					printf("\n%s\n",xbuff);
			} else {
				sjson_pl_t *p, *l;

				p = l = sjson_path( &tbl->tbl );
				while (l) {
					if (sjson_path_deleted(l))
						printf(" D");
					else
						printf("  ");
					if (sjson_path_prio(l))
						printf(" T");
					else 
						printf("  ");
					printf(" %-40s\n",l->path);
					l = sjson_path_next( l , 0 );
				}
				sjson_path_delete(p);
				res = 1;
			}

			printf("%s",!res?"Failed":"Succeeded");
			if (res) printf(" %d",index);
			printf("\n");
		} else	
		if (!strcasecmp(argv[0],"DELETE")) {
			char * path;
			sjson_node_t *ntree;

			/* deleting the selected tree */
			if (nth < 2)
				return 0;

			path = NULL;
			ntree = NULL;

			index = strtol( argv[1] , &eptr, 10 );
			if (!(index >= 0 && index < MAXTREE) || 
					(eptr == argv[1]))
				return 0;

			tbl = &(info[ index ]);
			if (!tbl->used) 
				return 0;
		
			if (nth >= 3) {
				int mode = 0; /* full compare */
				int xindex; 
				sjson_table_t *t = &(tbl->tbl);

				xindex = strtol( argv[2], &eptr, 10);
				if (!(xindex >= 0 && xindex < MAXTREE) || 
							(eptr == argv[2])) {
					path = argv[2];
				}

				if (nth >= 4) {
					if (!strcasecmp(argv[3],"LOOSE"))
						mode = 1;
				}

				if (path) {
					ntree = sjson_delete_json( t->root[0] , path , mode );
					res = ntree ? 0 : 1;
				} else {
					jtbl_info_t *xtbl;
					sjson_pl_t * proot, * ptrace;

					xtbl = &(info[ xindex ]);
					if (!xtbl)
						return 0;

					proot = ptrace = sjson_path( &(xtbl->tbl) );
					if (proot)  {
						while (ptrace) {
							if (sjson_path_prio(ptrace)) {
								sjson_delete_json( t->root[0], ptrace->path , mode );
							}
							ptrace = sjson_path_next( ptrace , 1 );
						}
						sjson_path_delete( proot );
						res = 0;
					}
				}
			} else {
				sjson_free( &tbl->tbl );
				res = jtbl_free( index );
			}

			printf("%s",res?"Failed":"Succeeded");
			if (!res) printf(" %d",index);
			printf("\n");
		} else	
		if (!strcasecmp(argv[0],"PURGE")) {
			int count;
			#define _MAXSUPR_ 5
			int lcnt;
			int tcnt;
			sjson_table_t *t;

			/* deleting the selected tree */
			if (nth < 2)
				return 0;

			index = strtol( argv[1] , &eptr, 10 );
			if (!(index >= 0 && index < MAXTREE) || 
					(eptr == argv[1]))
				return 0;

			tbl = &(info[ index ]);
			if (!tbl->used) 
				return 0;
	
			lcnt = 0;	
			tcnt = 0;
			t = &(tbl->tbl);
			do {
				count = sjson_purge_json( t->root[0] );
				tcnt += count;
				if (lcnt++ >= _MAXSUPR_)
					break;
			} while (count);

			printf("%d blocks purged\n",tcnt);
			printf(" %d\n",index);
		} else	
		if (!strcasecmp(argv[0],"ADD")) {
			jtbl_info_t *otbl;	
			jtbl_info_t *atbl;	
			sjson_node_t *tree;
			sjson_node_t *n; 
			sjson_node_t *a;
			sjson_pl_t * proot, * ptrace;
			int oindex, aindex;

			/* deleting the selected tree */
			if (nth < 3)
				return 0;

			tree = NULL;

			index = strtol( argv[1] , &eptr, 10 );
			if (!(index >= 0 && index < MAXTREE) || 
					(eptr == argv[1]))
				return 0;

			otbl = &(info[ index ]); /* original jason table */
			if (!otbl->used) 
				return 0;

			oindex = index;
	
			index = strtol( argv[2] , &eptr, 10 );
			if (!(index >= 0 && index < MAXTREE) || 
					(eptr == argv[1]))
				return 0;

			atbl = &(info[ index ]);  /* another json table to be merged */
			if (!atbl->used) 
				return 0;

			aindex = index;

			/* tree */
			n = otbl->tbl.root[0];
			a = atbl->tbl.root[0];

			if (!n || !a)
				return 0;

			/*
			 * set index rearrange
			 */
			sjson_precalc_setmember( &(otbl->tbl) );

			/*
			 *  n = n + a   -- The operation we would like to do 
			 */

			/* create path list from the second table */
			proot = ptrace = sjson_path( &(atbl->tbl) );
			if (!proot) 
				return 0;

			/* now adding... */
			ptrace = proot;
			while (ptrace) {
				sjson_table_t tbl;
				/* find a tree for the path */
				if (sjson_path_prio( ptrace)) {
					tree = sjson_find_json( a, ptrace->path , 0 );
					if (!tree) {
						printf("ERR:%s cannot build up tree\n",ptrace->path);
						break;
					}
					n = sjson_add_json( n , tree , ptrace->path ); /* adding two json trees */
					tbl.root[0] = tree;
					sjson_free( &tbl ); /* deleting the tree after being added */
				}
				ptrace = sjson_path_next( ptrace, 1 ); /* token node */
			}

			sjson_path_delete( proot );
			printf("Succeeded %d <- %d\n", oindex, aindex);
		} else	
		if (!strcasecmp(argv[0],"STATUS")) {
			int len;
			int ii;

			len = 0;
			memset(xbuff, 0, DSIZE);
			len += sprintf(xbuff+len,"[ ");
			for (ii = 0; ii < MAXTREE; ii++) {
				if (!info[ ii ].used) 
					continue;
				len += sprintf(xbuff+len,"%d ",ii);
			}
			len += sprintf(xbuff+len,"]  %d bytes", __os_memspc());
			printf("%s\n",xbuff);

			if (nth >= 2) {
				index = strtol( argv[1] , &eptr, 10 );
				if ((index != 1) || (eptr == argv[1]))
					return 0;

				__os_memblks( dump_mem_block ); /* print out memory fragments still in use */
			}
		} else	
		if (!strcasecmp(argv[0],"TREE")) {
			char *tbuff;
			int amt, tsize;
			char *outfn;

			if (nth < 2)
				return 0;

			index = strtol( argv[1] , &eptr, 10 );
			if (!(index >= 0 && index < MAXTREE) ||
					(eptr == argv[1]))
				return 0;

			tbl = &(info[ index ]);
			if (!tbl->used) 
				return 0;

			amt = 1; /* amt x 1MBytes */
			if (nth >= 3) {
				amt = strtol( argv[2] , &eptr, 10 );
				if (!(amt >= 1 && amt <= 64) || (eptr == argv[2]))
					return 0;
			}

			outfn = NULL;
			if (nth >= 4)
				outfn = argv[3];

			/* CAUTION !! - system APIs are used malloc/free */
			#define TBUFSZ (64*1024)
			tsize = amt * TBUFSZ;
			tbuff = (char *)malloc( tsize );
			if (!tbuff)
				return 0;
			memset( tbuff, 0, tsize );
			res = sjson_print_tree( &tbl->tbl , tbuff, tsize-32 );
			if (res < 0) {
				printf("not enough buffer \n");
				free(tbuff);
				return 0;
			}
			if (res > 0) {
				if (outfn) {
					int fd;

					/* file writing !! */
					fd = open(outfn,O_RDWR|O_CREAT,0666);
					if (fd > 0) {
						write(fd, xbuff, res);
						close(fd);
					}
				} else {
					printf("%s\n",tbuff);
				}
			}
			free(tbuff);
		} else	
		if (!strcasecmp(argv[0],"DBG")) {
			index = 0;

			if (nth == 2) {
				/* index is debugging level */
				index = strtol( argv[1] , &eptr, 10 );
				if (!(index >= 0 && index <= 2) ||
						(eptr == argv[1]))
					return 0;
			}

			if (index == 0) {
				json_dbg = (json_dbg == 1) ? 0 : 1;
			} else
			if (index == 1) {
				mem_dbg = (mem_dbg == 1) ? 0 : 1;
			} else 
			if (index == 2) {
				json_dbg = (json_dbg == 1) ? 0 : 1;
				mem_dbg = (mem_dbg == 1) ? 0 : 1;
			}
			printf("%d %d\n",json_dbg,mem_dbg);
		} else	
		if (!strcasecmp(argv[0],"RUN")) {
			char * fn;
			FILE *fp;
			char cmd[INPUTLEN];
			char _cmd[INPUTLEN];
			char *xargs[MAXARGS];
			int nnth;
			int done;

			if (!(nth >= 2))
				return 0;

			fn = argv[1]; /* file name */

			fp = fopen(fn,"ro");
			if (!fp)
				return 0;

			done = 0;
			while (!done) {
				memset(cmd,0,INPUTLEN);
				memset(_cmd,0,INPUTLEN);
				if ( !fgets(cmd,INPUTLEN,fp) )
					break;
				strcpy(_cmd,cmd);
				_cmd[ strlen(_cmd)-1 ] = 0x0;
				if ( parse_cmd( cmd, xargs, &nnth ) == 0 ) {
					if (cmd[0] == '#')
						continue; /* next line */
					if (!strcasecmp( xargs[0], "RUN" ))  /* infinite loop */
						break; 
					printf("\nCMD: <%s>\n",_cmd);
					done = process_commands( xargs , nnth );
				}
			}
			fclose(fp);
		} else	
		if (!strcasecmp(argv[0],"QUIT") ||
				!strcasecmp(argv[0],"EXIT")) {
			return -1;
		} 
	} while (0);

	return 0;
}

int main(int argc,char *argv[])
{
	char cmds[INPUTLEN];
	char *args[MAXARGS];
	int done;
	int ii, /* index, count, */ nth;  

	__os_meminit( 4*1024*1024 ); /* memory allocation */

	for (ii = 0; ii < MAXTREE; ii++)
		info[ ii ].used = 0;

	if (argc > 1) {
		int cnt = 0;
		for (cnt = 1; cnt < argc; cnt++ ) {
			args[0] = "run";
			args[1] = argv[cnt++];		
			done = process_commands( args , 2 );
		}
	}

	done = 0;
	while (!done) {
		printf("[JSON] "); 
		memset(cmds,0,INPUTLEN);
		if (!fgets(cmds,INPUTLEN-1,stdin))
			continue;
	
		if ( parse_cmd( cmds, args, &nth ) == 0 )
			done = process_commands( args , nth );
	}
	
}

