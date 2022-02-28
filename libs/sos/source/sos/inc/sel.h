/*******************************************************************************
 *
 * sel.h - selector module
 *
 ******************************************************************************/
#ifndef _NETWORK_SELECTOR_MODULE_HEADER
#define _NETWORK_SELECTOR_MODULE_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define NET_SEL_MAX_SIZE 32

/* select group */
typedef struct {
	fd_set rfds;       /* read  original copy */
	fd_set wfds;       /* write original copy */
	fd_set res_rfds;   /* result */
	fd_set res_wfds;   /* result */
	unsigned int maxfd; /* max fd    */
	unsigned int num;  /* size of list */
	unsigned int member[NET_SEL_MAX_SIZE];
}net_sel_group;

/* prototypes */
extern unsigned int net_port_proto_select_cleanup(net_sel_group *selg);
extern unsigned int net_port_proto_select_group(net_sel_group *selg,...);
extern unsigned int net_port_proto_select_delete(net_sel_group *selg,unsigned int portid);
#define NET_READ_SELECT 0x00FF0001
#define NET_WRITE_SELECT    0x00FF0002
#define NET_RW_SELECT   0x00FF0003
extern int          net_port_proto_select(net_sel_group *selg,unsigned int mode,unsigned long tm);
extern int          net_port_proto_select_test(net_sel_group *selg,unsigned int mode,unsigned int portid);

#ifdef __cplusplus
}
#endif

#endif /* UTVSW_NETWORK_SELECTOR_MODULE_HEADER */


