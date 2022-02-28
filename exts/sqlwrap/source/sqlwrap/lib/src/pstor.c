#include <stdio.h>
#include <stdlib.h>
#include "oskit.h"
#include "dblib.h"

#define DB_SPACE_COLUMN_DELIMITER  0x5a5a9821

typedef struct {
	int ret;
  #define DBMAN_ERRMSG_LEN  64
	char msg[DBMAN_ERRMSG_LEN];
}__attribute__((packed)) dbman_item_result_header_t;

typedef struct {

  #define DB_SPACE_TYPE_ITEM   0x01
  #define DB_SPACE_TYPE_COLUMN 0x02
  #define DB_SPACE_TYPE_HEADER 0x03
  #define DB_SPACE_TYPE_RESULT_HEADER 0x04
	unsigned char type;    /* type ??? */

  #define DB_SPACE_TYPE_ITEM_INTEGER   0xE1
  #define DB_SPACE_TYPE_ITEM_STRING    0xE2
	unsigned char subtype; /* integer/string... */

	unsigned int row;      /* Row + */
	unsigned int col;      /* Col +- only meaningful when type==HEADER */
}__attribute__((packed)) dbman_item_header_t;

/*
 * Decrypting...
 *
 * char space -> result_list
 *
 */
db_result_list * dbman_sql_decrypt_result(unsigned char *space,unsigned int space_sz)
{
	int ix,iy;
	unsigned int current_offset;
	unsigned int delim;
	int row,col,item_row,item_col;
	db_result_list        *result;
	db_result_list_column *line,*line_root,*line_current;
	db_result_list_item   *item, *current_item;
	dbman_item_header_t   *hdr;
	dbman_item_result_header_t   *reshdr;

	if(!space || !space_sz) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt_result::null data\n");
		return NULL;
	}

	hdr = (dbman_item_header_t *)space;
	if(hdr->type != DB_SPACE_TYPE_HEADER) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt_result::Header type broken\n");
		return NULL;
	}

	row = ntohl(hdr->row); /* row */
	col = ntohl(hdr->col); /* col */

	if(row<0 || col<0) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt_result::row=%d,col=%d::sanity error\n",row,col);
		return NULL;
	}
	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_decrypt_result::row=%d,col=%d\n",row,col);

	/* starting offset */
	current_offset = sizeof(dbman_item_header_t) + sizeof(dbman_item_result_header_t);

	/* initialize trace... */
	line_root = line_current = NULL;

	/* scanning data ... */
	for(ix=0; ix<row; ix++) {
		delim = *(unsigned int *)(space + current_offset);
		if(ntohl(delim) != DB_SPACE_COLUMN_DELIMITER) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt::Wrong column delimiter!! at row=%d,col=%d\n",ix,iy);
			goto __fail_decrypt;
		}
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_decrypt::Column at %d\n",current_offset);
		current_offset += sizeof(unsigned int);

		/* allocating item list... */
		current_item = (db_result_list_item *)db_mem_alloc(sizeof(db_result_list_item)*col);
		if(!current_item) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt::db_mem_alloc(item*%d):;failed\n",col);
			goto __fail_decrypt;
		}

		for(iy=0; iy<col; iy++) {
			hdr = (dbman_item_header_t *)(space+current_offset);
			dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_decrypt::Item at %d\n",current_offset);
			if(hdr->type != DB_SPACE_TYPE_ITEM) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt_result::Item type broken!!\n");
				dbman_printf(DBMAN_DBG_LVL_ERR,"hdr->type    = %02x\n",hdr->type);
				dbman_printf(DBMAN_DBG_LVL_ERR,"hdr->subtype = %02x\n",hdr->subtype);
				goto __fail_decrypt;
			}
			/* increment offset */
			current_offset += sizeof(dbman_item_header_t);

			/* header sanity check */
			item_row = ntohl(hdr->row);
			item_col = ntohl(hdr->col);
			if(item_row!=ix || item_col!=iy) {
				dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt_result::index value broken(R=%d,IR=%d)(C=%d,IC=%d)\n",
				             ix,item_row,iy,item_col);
				goto __fail_decrypt;
			}

			/* build up item node */
			switch(hdr->subtype) {
			case DB_SPACE_TYPE_ITEM_INTEGER:
				current_item[iy].type = DB_TYPE_INTEGER;
				current_item[iy].intval = ntohl(*(unsigned int *)(space+current_offset));
				dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_decrypt::Item(Int) at %d\n",current_offset);
				current_offset += sizeof(unsigned int);
				break;
			case DB_SPACE_TYPE_ITEM_STRING:
				current_item[iy].type = DB_TYPE_STRING;
				current_item[iy].lenstr = ntohl(*(unsigned int *)(space+current_offset));
				dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_decrypt::Item(Str) at %d\n",current_offset);
				current_item[iy].strval = (unsigned char *)db_mem_alloc(current_item[iy].lenstr);
				if(!current_item[iy].strval) {
					dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt_result::db_mem_alloc(lenstr=%d)::error\n",
					             current_item[iy].lenstr);
					goto __fail_decrypt;
				}
				current_offset += sizeof(unsigned int);
				memcpy( current_item[iy].strval,
				        (unsigned char *)(space+current_offset),
				        current_item[iy].lenstr);
				current_offset += current_item[iy].lenstr;
				break;
			default:
				break;
			}
		} /* for(iy... */

		/* creating a column... */
		line = (db_result_list_column *)db_mem_alloc(sizeof(db_result_list_column));
		if(!line) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt_result::db_mem_alloc(..result_list_column)::error\n");
			goto __fail_decrypt;
		}
		line->count = col;
		line->items = current_item;
		line->next = NULL;

		/* linking column list */
		if(!line_root) {
			line_root = line_current = line;
		}else{
			line_current->next = line;
			line_current = line;
		}
	} /* for(ix... */

	/* result node */
	result = (db_result_list *)db_mem_alloc(sizeof(db_result_list));
	if(!result) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt_result::db_mem_alloc(,,db_result_list)::error\n");
		goto __fail_decrypt;
	}

	result->count = row;
	result->items = line_root;

	/* return value processing */
	reshdr = (dbman_item_result_header_t *)(space + sizeof(dbman_item_header_t));
	result->ret = ntohl(reshdr->ret);
	if(result->ret) {
		result->errmsg = (unsigned char *)db_mem_alloc(DBMAN_ERRMSG_LEN);
		if(!result->errmsg) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_decrypt_result::db_mem_alloc(..ERRMSG_LEN)::error\n");
			goto __fail_decrypt;
		}
		memcpy(result->errmsg,reshdr->msg,DBMAN_ERRMSG_LEN);
	}else
		result->errmsg = NULL;

	return result;

__fail_decrypt:

	/* resource cleanup */
	if(result) db_mem_free(result);
	while(line_root) {
		line_current = line_root;
		line_root = line_root->next;
		for(ix=0; ix<line_current->count; ix++) {
			item = (db_result_list_item *)&(line_current->items[ix]);
			if(item) {
				if(item->type == DB_TYPE_STRING) {
					db_mem_free(item->strval);
				}
			}
		}
		db_mem_free(line_current);
	}
	return NULL;
}

/*
 * inserting a header .....
 */
static unsigned int dbman_sql_insert_header(
	unsigned char *space,
	unsigned int space_sz,
	dbman_item_header_t *hdr,
	unsigned int type,
	unsigned int current_offset,
	void *data)
{
	db_result_list        *result;
	db_result_list_column *line;
	db_result_list_item   *item;
	dbman_item_result_header_t *errmsg;
	unsigned int amount;

	/* sanity check */
	if(!data) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_insert_header::null data\n");
		return 0;
	}

	/* expected amount of data to be sliced... */
	amount = 0;

	/* calculate current data size */
	switch( type ) {
	case DB_SPACE_TYPE_COLUMN:
		amount = sizeof(unsigned int); /* delimiter */
		break;
	case DB_SPACE_TYPE_ITEM:
		item = (db_result_list_item *)data;
		switch(item->type) {
		case DB_TYPE_INTEGER:
			amount = sizeof(dbman_item_header_t) + sizeof(unsigned int);     /* header + delimiter */
			break;
		case DB_TYPE_STRING:
			amount = sizeof(dbman_item_header_t) + item->lenstr + sizeof(unsigned int);
			/* header + delimiter */
			break;
		default:
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_insert_header::unknown item type=%08x\n",item->type);
			return 0;
		}
		break;
	case DB_SPACE_TYPE_HEADER:
		amount = sizeof(dbman_item_header_t); /* header */
		break;
	case DB_SPACE_TYPE_RESULT_HEADER:
		amount = sizeof(dbman_item_result_header_t); /* result header */
		break;
	default:
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_insert_header::unknown type=%08x\n",type);
		return 0;
	}

	/* available space check */
	if((amount + current_offset) > space_sz) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_insert_header::too small space...=%08x,current=%08x\n",
		             space_sz,amount + current_offset);
		return 0;
	}

	/* inserting data */
	dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_insert_header::type=%08x\n",type);
	switch( type ) {
	case DB_SPACE_TYPE_COLUMN:
		/* host <-> network compatibility */
		*(unsigned int *)(space + current_offset) =
			htonl(DB_SPACE_COLUMN_DELIMITER);
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_insert_header::(TYPE_COLUMN...)done with(%d)\n",current_offset);
		break;
	case DB_SPACE_TYPE_ITEM:
		item = (db_result_list_item *)data;
		hdr->col  = htonl(hdr->col);
		hdr->row  = htonl(hdr->row);
		hdr->type = type;
		hdr->subtype = (item->type==DB_TYPE_INTEGER) ?
		               DB_SPACE_TYPE_ITEM_INTEGER : DB_SPACE_TYPE_ITEM_STRING;
		memcpy( (unsigned char *)(space + current_offset),
		        (unsigned char *)hdr,
		        sizeof(dbman_item_header_t));
		switch(item->type) {
		case DB_TYPE_INTEGER:
			*(unsigned int *)(space +
			                  current_offset +
			                  sizeof(dbman_item_header_t)) = htonl(item->intval);
			dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_insert_header::(TYPE_ITEM::INTEGER...)done with(%d)\n",
			             current_offset+sizeof(dbman_item_header_t));
			break;
		case DB_TYPE_STRING:
			*(unsigned int *)(space +
			                  current_offset +
			                  sizeof(dbman_item_header_t)) = htonl(item->lenstr);
			memcpy( (unsigned char *)(space +
			                          current_offset +
			                          sizeof(dbman_item_header_t) +
			                          sizeof(unsigned int)),
			        (unsigned char *)(item->strval),
			        item->lenstr);
			dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_insert_header::(TYPE_ITEM::STRING...)done with(%d)\n",
			             current_offset+sizeof(dbman_item_header_t)+sizeof(unsigned int));
			break;
		}
		break;
	case DB_SPACE_TYPE_HEADER:
		/* host <-> network compatibility */
		hdr->col  = htonl(hdr->col);
		hdr->row  = htonl(hdr->row);
		hdr->type = type;
		memcpy( (unsigned char *)(space + current_offset),
		        (unsigned char *)hdr,
		        sizeof(dbman_item_header_t));
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_insert_header::(TYPE_HEADER...)done with(%d)\n",current_offset);
		break;
	case DB_SPACE_TYPE_RESULT_HEADER:
		result = (db_result_list *)data;
		if(!result) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_insert_header::null result parameter\n");
			return current_offset + amount;
		}
		errmsg = (dbman_item_result_header_t *)(space + current_offset);
		memset(errmsg->msg,0x00,DBMAN_ERRMSG_LEN);
		if(result->errmsg) memcpy(errmsg->msg, result->errmsg, DBMAN_ERRMSG_LEN);
		errmsg->ret = htonl(result->ret);
		dbman_printf(DBMAN_DBG_LVL_MSG,"dbman_sql_insert_header::(TYPE_RESULT...)done with(%d)\n",current_offset);
		break;
	}

	/* next offset */
	return current_offset + amount;
}

/*
 * Enctrypting...
 *
 * result_list -> char space
 *
 */
int dbman_sql_encrypt_result(unsigned char *space,unsigned int size,db_result_list *result)
{
	unsigned char *memp;
	unsigned char *next_p;
	int ix,iy,rows,csz,sz;
	unsigned int space_current_offset;
	db_result_list_column *line;
	db_result_list_item   *item;
	dbman_item_header_t   *hdr;

	if(!result) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_encrypt_result::NO result\n");
		return (-1);
	}

	memp = space;
	if(!memp) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_encrypt_result::db_mem_alloc::error\n");
		return (-1);
	}

	/* header creation... */
	hdr = (dbman_item_header_t *)db_mem_alloc(sizeof(dbman_item_header_t));
	if(!hdr) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_encrypt_result::db_mem_alloc(...item_header_t)::error\n");
		return (-1);
	}

	/*
	 * header is prepended at the end of this function...
	 */
	space_current_offset = sizeof(dbman_item_header_t) + sizeof(dbman_item_result_header_t);

	csz = 0;
	rows = dbman_result_list_size(result); /* How may rows... */
	for(ix=0; ix<rows; ix++) {
		line = dbman_fetch_column(result,ix); /* How many columns... */
		if(!line) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"error... no line!!\n");
			goto __end;
		}
		if(!space_current_offset) {
			dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_encrypt_result::space_pointer=0::error\n");
			goto __end;
		}
		csz = dbman_result_column_size(line);
		/* adding up column data */
		hdr->row = ix;
		hdr->col = csz;
		space_current_offset = dbman_sql_insert_header(
			memp, /* address space */
			size,
			hdr,
			DB_SPACE_TYPE_COLUMN,
			space_current_offset,
			line);
		if(csz) {
			for(iy=0; iy<csz; iy++) {
				item = dbman_fetch_item(line,iy);
				if(!item) {
					dbman_printf(DBMAN_DBG_LVL_ERR,"error... no item!!\n");
					goto __end;
				}
				switch(DB_ITEM_TYPE(item)) {
				case DB_TYPE_INTEGER:
					/* adding up item data */
					hdr->row = ix;
					hdr->col = iy;
					space_current_offset = dbman_sql_insert_header(
						memp, /* address space */
						size,
						hdr,
						DB_SPACE_TYPE_ITEM,
						space_current_offset,
						(char *)item);
					if(!space_current_offset) {
						dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_encrypt_result::space_pointer=0::item(int)::error\n");
						goto __end;
					}
					dbman_printf(DBMAN_DBG_LVL_MSG,"value = %d , ",DB_ITEM_INTVAL(item));
					break;
				case DB_TYPE_STRING:
					/* adding up item data */
					hdr->row = ix;
					hdr->col = iy;
					space_current_offset = dbman_sql_insert_header(
						memp, /* address space */
						size,
						hdr,
						DB_SPACE_TYPE_ITEM,
						space_current_offset,
						(char *)item);
					if(!space_current_offset) {
						dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_encrypt_result::space_pointer=0::item(str)::error\n");
						goto __end;
					}
					dbman_printf(DBMAN_DBG_LVL_MSG,"value = %s , ",DB_ITEM_STRVAL(item));
					break;
				default:
					break;
				}
			}
			dbman_printf(DBMAN_DBG_LVL_MSG,"\n");
		}
	}
	/* prepending header data */
	hdr->row = rows;
	hdr->col = csz;
	space_current_offset = dbman_sql_insert_header(
		memp, /* address space */
		size,
		hdr,
		DB_SPACE_TYPE_HEADER,
		0,
		(char *)result);
	if(!space_current_offset) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_encrypt_result::space_pointer=0::header::error\n");
		goto __end;
	}

	/* Error message next */
	space_current_offset = dbman_sql_insert_header(
		memp, /* address space */
		size,
		NULL,
		DB_SPACE_TYPE_RESULT_HEADER,
		sizeof(dbman_item_header_t),
		(char *)result);

	if(!space_current_offset) {
		dbman_printf(DBMAN_DBG_LVL_ERR,"dbman_sql_encrypt_result::space_pointer=0::result::error\n");
		goto __end;
	}

	db_mem_free(hdr);

	return 0; /* Success */

__end:

	db_mem_free(hdr);

	return (-1); /* Failure */
}

