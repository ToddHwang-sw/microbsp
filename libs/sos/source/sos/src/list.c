/*******************************************************************************
 *
 * list.c - List container and manipulation
 *
 ******************************************************************************/
#include "oskit.h"

/* private functions */
static unsigned int _list_item_insert(OS_LIST *h,OS_LIST_DATA *d);
static unsigned int _list_item_delete(OS_LIST *h, OS_LIST_DATA *t);

/*******************************************************************************
 *
 * os_list_create - Creating a list for OS resource management
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  name - list name
 *
 * RETURNS
 *  NULL - Failed.
 *  !NULL - Success
 *
 * ERRORS
 *  N/A
 */
OS_LIST * os_list_create(const char *name)
{
	OS_LIST *os_c;

	/* os_c block */
	os_c = (OS_LIST *)os_mem_alloc( sizeof( OS_LIST ) );
	if(!os_c) {
		return NULL;
	}
	strncpy( (char *)os_c->name, (const char *)name, STR_LEN ); /* name */
	os_c->data = NULL;                /* data */
	_os_mutex_init( (&os_c->mut) );       /* mutex */
	os_c->counter = 0;                /* counter */

	return os_c;
}

/*******************************************************************************
 *
 * os_list_delete - Deleting a list for OS resource management
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - list
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int os_list_delete(OS_LIST *h)
{
	unsigned int notyetdeleted;
	OS_LIST_DATA *d, *p;

	_os_mutex_lock(&(h->mut));

	p = d = h->data;

	/* here, we checks up any items not to be deleted.
	   It may cause memory leak. */
	notyetdeleted = 0;
	while(d) { /* backward traverse */
		d = d->next;
		if( p->data != NULL ) {
			notyetdeleted = 1;
		}

		/* free up resource ID */
		if( os_res_id_free(h,p->resid) == ERROR ) {
			os_debug_printf(OS_DBG_LVL_ERR,"os_res_id_free(%s,%08x)::error\n",
			                h->name,p->resid);
		}

		os_mem_free(p);
		p = d;
	}
	_os_mutex_unlock(&(h->mut));

	/* any items found not to be deleted */
	if(notyetdeleted) {
		os_debug_printf(OS_DBG_LVL_ERR,"Items in list (%s) seems not to be cleanly deleted\n",h->name);
		// return ERROR;
	}

	os_mem_free(h); /* header block free */

	return OKAY;
}

/*******************************************************************************
 *
 * os_list_flush - Flushing a list for OS resource management
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - list
 *  f - delete function
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
unsigned int os_list_flush(OS_LIST *h,os_delfunc_t delf)
{
	unsigned int res;
	OS_LIST_DATA *d, *p;

	_os_mutex_lock(&(h->mut));

	p = d = h->data;

	while(d) { /* backward traverse */
		d = d->next;
		if(p->data != NULL) {
			if(delf) {
				res = delf(p->data); /* delete function */
				p->data = (res==OKAY) ? NULL : p->data;
				if(p->data!=NULL) {
					os_debug_printf(OS_DBG_LVL_ERR,"list flush error\n");
				}
			}
		}

		/* free up resource ID */
		if( os_res_id_free(h,p->resid) == ERROR ) {
			os_debug_printf(OS_DBG_LVL_ERR,"os_res_id_free(%s,%08x)::error\n",
			                h->name,p->resid);
		}

		os_mem_free(p);
		p = d;
	}
	_os_mutex_unlock(&(h->mut));
	h->data = NULL;
	h->counter = 0;
	return OKAY;
}

/*******************************************************************************
 *
 * os_list_find - Finding an item from a list
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - list
 *  comp - comparator
 *  arg - argument
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
OS_LIST_DATA * os_list_find(OS_LIST *h,
                            unsigned int (*comp)(OS_LIST_DATA *,void *),void *arg)
{
	OS_LIST_DATA *d;

	d = h->data;

	while(d) {
		if( (unsigned int)(*comp)(d,arg) == OKAY )
			return d;
		d = d->next;
	}

	return NULL;
}

/*******************************************************************************
 *
 * os_list_navigate - Navigating a list
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - list
 *  action - action function
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
void os_list_navigate(OS_LIST *h,
                      void (*haction)(OS_LIST *),
                      void (*action)(OS_LIST_DATA *))
{
	OS_LIST_DATA *d;

	if(!h) return;

	_os_mutex_lock( &(h->mut) );
	if(haction) (*haction)(h);

	d = h->data;
	while(d) { if(action) (*action)(d); d = d->next; }
	_os_mutex_unlock( &(h->mut) );
}

/* static function */
static void os_list_verbose_item(OS_LIST_DATA *p)
{
	printf("Items ... [%08x]\n",p->resid);
}

/*******************************************************************************
 *
 * os_list_item_insert - Inseting a data into a list
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of list
 *  data - data itself
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
unsigned int os_list_item_insert(OS_LIST *h,void *data)
{
	OS_LIST_DATA *d; /* new instance */

	d = (OS_LIST_DATA *)os_mem_alloc(sizeof(OS_LIST_DATA));
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%s)ITEM_INSERT(E1)\n",h->name);
		return ERROR;
	}
	d->data = data; /* real data */
	d->prev =
		d->next = NULL;
	d->key = OS_LIST_NO_KEY; /* Se-Jin */

	return _list_item_insert(h, d);
}

/*******************************************************************************
 *
 * os_list_item_delete - Inseting a data into a list
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of list
 *  id - resource Id
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
unsigned int os_list_item_delete(OS_LIST *h,unsigned int id)
{
	OS_LIST_DATA *t; /* trace */

	t = h->data; /* first thing */
	if(!t) {
		return ERROR;
	}

	while(t && t->resid!=id) {
		t = t->next;
	}
	return _list_item_delete(h, t);
}

/*******************************************************************************
 *
 * os_list_item_pop - Poping a data from the list
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of list
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
void * os_list_item_pop(OS_LIST *h)
{
	OS_LIST_DATA *t, *p; /* trace */
	void * data;

	t = h->data; /* first thing */
	if(!t) {
		return NULL;
	}

	if(!t->data) {
		/* inconsistent */
		os_debug_printf(OS_DBG_LVL_ERR,"(%s)ITEM_POP(E1)\n",h->name);
		return NULL;
	}

	_os_mutex_lock(&(h->mut));

	data = t->data; /* real data */

	h->data = t->next;
	p = t->next;
	if(p) p->prev = NULL; /* link chaining */

	/* free up resource ID */
	if( os_res_id_free(h,t->resid) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_res_id_free(%s,%08x)::error\n",
		                h->name,t->resid);
	}

	os_mem_free(t);

	--(h->counter);

	_os_mutex_unlock(&(h->mut));

	return data;
}

/*******************************************************************************
 *
 * os_list_sanity_check - Sanity checking a list
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of list
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
unsigned int os_list_item_sanity_check(OS_LIST *h)
{
	unsigned int ix; /* loop */
	unsigned int c;
	OS_LIST_DATA *d, *p, *n;

	if(!h) {
		return ERROR;
	}
	_os_mutex_lock( &(h->mut) );

	c = h->counter;
	d = h->data;
	for(ix=0; ix<c && d; ix++) {
		if( !(d->data) ) {
			os_debug_printf(OS_DBG_LVL_ERR,"(%s)SAN(E1)\n",h->name);
			_os_mutex_unlock( &(h->mut) );
			return ERROR;
		}
		switch(ix) {
		case 0: /* first node */
			if(d->prev != h->data) {
				os_debug_printf(OS_DBG_LVL_ERR,"(%s)SAN(E2)\n",h->name);
				_os_mutex_unlock( &(h->mut) );
				return ERROR;
			}
			break;
		default: /* in the middle of list */
			n = d->next;
			p = d->prev;
			if(n) { /* next and prev check */
				if((n->prev!=d)||(p->next!=d)) {
					os_debug_printf(OS_DBG_LVL_ERR,"(%s)SAN(E3)\n",h->name);
					_os_mutex_unlock( &(h->mut) );
					return ERROR;
				}
			}else{ /* last node */
				if(p->next!=d) {
					os_debug_printf(OS_DBG_LVL_ERR,"(%s)SAN(E4)\n",h->name);
					_os_mutex_unlock( &(h->mut) );
					return ERROR;
				}
			}
			break;
		} /* switch */
	} /* for */

	_os_mutex_unlock( &(h->mut) );
	return OKAY;
}

/*******************************************************************************
 *
 * os_list_findbykey - Finding an item by its key
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - list
 *  key: key to find the data
 *
 * RETURNS
 *  OKAY/ERROR
 *
 * ERRORS
 *  N/A
 */
void * os_list_findbykey(OS_LIST *h, long key)
{
	OS_LIST_DATA *d;

	d = h->data;

	while(d) {
		if( d->key == key) {
			return d->data;
		}
		d = d->next;
	}

	return NULL;
}


/*******************************************************************************
 *
 * os_list_item_insert_order - Inserting a data into a list by order of the keys
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of list
 *  data - data itself
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
unsigned int os_list_item_insert_order(OS_LIST *h,long key, void *data)
{
	unsigned int resid; /* resource ID */
	OS_LIST_DATA *d; /* new instance */
	OS_LIST_DATA *t; /* for tracing */
	unsigned int trace_t; /* verification */

	os_debug_printf(OS_DBG_LVL_VER,"dispatch handler :: malloc olist data key = %ld\n", key);

	d = (OS_LIST_DATA *)os_mem_alloc(sizeof(OS_LIST_DATA));
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"(%s)ITEM_INSERT(E1)\n",h->name);
		return ERROR;
	}
	d->data = data; /* real data */
	d->key = key;
	d->prev =
		d->next = NULL;

	_os_mutex_lock(&(h->mut));

	/* resource ID generation */
	resid = os_res_id_alloc(h);
	if(!resid) {
		_os_mutex_unlock(&(h->mut));
		os_debug_printf(OS_DBG_LVL_ERR,"(%s)ITEM_INSERT(E2)\n",h->name);
		return ERROR;
	}

	if(!h->counter) {
		/* empty olist */
		h->data = d; /* first & last data */
	}else{
		t = h->data;
		trace_t = 0;
		if (t->key > key) {
			d->next = t;
			t->prev = d;
			h->data = d;
			goto finish_insert;
		}
		while(t->next && (trace_t<h->counter) && (t->next->key <= key)) {
			t=t->next; /* go to the last node */
			++trace_t; /* for verification */
		}
		d->next = t->next;
		if (d->next != NULL)
			d->next->prev = d;
		t->next = d;
		d->prev = t; /* link chaining */
	}
finish_insert:

	/* error... */
	if( os_res_id_make_inuse(h,resid) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_res_id_alloc(%s,%08x)::error\n",
		                h->name,resid);
		return ERROR;
	}

	d->resid = resid; /* resource ID */

	++(h->counter); /* resource added */

	_os_mutex_unlock(&(h->mut));

	return resid;
}


/*******************************************************************************
 *
 * os_list_item_deletebykey - remove a data from a list by key
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of olist
 *  key - key to delete
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
unsigned int os_list_item_deletebykey(OS_LIST *h,long key)
{
	OS_LIST_DATA *t; /* trace */

	t = h->data; /* first thing */
	if(!t) {
		return ERROR;
	}

	while(t && t->key != key) {
		t = t->next;
	}
	return _list_item_delete(h, t);
}

/*******************************************************************************
 *
 * os_list_firstkey - get the first key of an ordered list
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of olist
 *
 * RETURNS
 *  NULL/key
 *
 * ERRORS
 *  N/A
 */
long os_list_firstkey(OS_LIST *h)
{
	OS_LIST_DATA *d;

	if (!h) {
		return (long)-1;
	}
	d = h->data;
	if (d) {
		return d->key;
	}

	return (long)-1;
}

/*******************************************************************************
 *
 * os_list_item_insert_hash - Inseting a data with key, can use like a hashtable
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of list
 *  key - key of the data, we use only long key here
 *  data - data itself
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
unsigned int os_list_item_insert_hash(OS_LIST *h,long key, void *data)
{
	OS_LIST_DATA *d; /* new instance */

	d = (OS_LIST_DATA *)os_mem_alloc(sizeof(OS_LIST_DATA));
	if(!d) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_mem_alloc(%s)::error\n",h->name);
		return ERROR;
	}
	d->data = data; /* real data */
	d->key = key;
	d->prev =
		d->next = NULL;

	return _list_item_insert(h, d);
}

/*******************************************************************************
 *
 * _list_item_insert - private function, Inseting a node into list
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of list
 *  d - the node
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
static unsigned int _list_item_insert(OS_LIST *h,OS_LIST_DATA *d)
{
	unsigned int resid; /* resource ID */
	OS_LIST_DATA *t; /* for tracing */
	unsigned int trace_t; /* verification */

	if(!h || !d) return ERROR;

	_os_mutex_lock(&(h->mut));

	/* resource ID generation */
	resid = os_res_id_alloc(h);
	if(!resid) {
		_os_mutex_unlock(&(h->mut));
		os_debug_printf(OS_DBG_LVL_ERR,"(%s)ITEM_INSERT(E2)\n",h->name);
		return ERROR;
	}

	if(!h->counter) {
		/* empty list - first | last data */
		h->data = d;
	}else{
		t = h->data;
		trace_t = 0;
		while(t->next && (trace_t<h->counter)) {
			/* tests if any of objects has the same res ID */
			/* system is keeping too many resources */
			if( t->resid == resid ) {
				os_debug_printf(OS_DBG_LVL_ERR,"Too many resources in (%s) pool\n",h->name);
				_os_mutex_unlock(&(h->mut));
				os_list_navigate(h,NULL,os_list_verbose_item);
				return ERROR;
			}
			t=t->next; /* go to the last node */
			++trace_t; /* for verification */
		}
		t->next = d;
		d->prev = t; /* link chaining */
	}

	/* error... */
	if( os_res_id_make_inuse(h,resid) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_res_id_alloc(%s,%08x)::error\n",
		                h->name,resid);
		return ERROR;
	}

	d->resid = resid; /* resource ID */

	++(h->counter); /* resource added */

	_os_mutex_unlock(&(h->mut));

	return resid;
}

/*******************************************************************************
 *
 * _list_item_delete - private function, to delete a node from list
 *
 * DESCRIPTION
 *
 * PARAMETERS
 *  h - Root node of list
 *  t - node to delete
 *
 * RETURNS
 *  ERROR/OKAY
 *
 * ERRORS
 *  N/A
 */
static unsigned int _list_item_delete(OS_LIST *h, OS_LIST_DATA *t)
{
	OS_LIST_DATA *p; /* trace */

	if(!t) { /* end of list */
		os_debug_printf(OS_DBG_LVL_ERR,"No resource has been found in (%s)\n",h->name);
		return ERROR;
	}

	if(!t->data) {
		/* inconsistent */
		os_debug_printf(OS_DBG_LVL_ERR,"Data block is NULL(%s)\n",h->name);
		return ERROR;
	}

	_os_mutex_lock(&(h->mut));

	/* previous pointer handling */
	if(t->prev) {
		p = t->prev;
		p->next = t->next;
	}else{
		if(h->data == t) {  /* if first node */
			h->data = t->next;
			p = t->next;
			if(p) p->prev = NULL;
		}
	}

	/* next pointer handling */
	if(t->next) {
		p = t->next;
		p->prev = t->prev;
	}

	/* free up resource ID */
	if( os_res_id_free(h,t->resid) == ERROR ) {
		os_debug_printf(OS_DBG_LVL_ERR,"os_res_id_free(%s,%08x)::error\n",
		                h->name,t->resid);
	}

	os_mem_free(t);

	--(h->counter);

	_os_mutex_unlock(&(h->mut));

	return OKAY;
}
