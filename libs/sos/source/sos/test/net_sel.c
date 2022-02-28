#include "oskit.h"

unsigned int pa,pb,pc,pd;
static stb_net_addrinfo_t addr;
static stb_net_sel_group selg;

static void local_print_select_group(stb_net_sel_group *selg)
{
	int ii;

	printf("group info = ");
	for(ii=0; ii<selg->num; ii++) {
		printf("<%08x> ",selg->member[ii]);
	}
	printf("\n");
}

#ifdef _WIN32_WCE
int _tmain(int argc,char *argv[])
#else
main()
#endif
{
	stb_os_init();

	stb_net_addrinfo_make(&addr,STB_NET_IP(192,168,2,13),1000);
	pa = stb_net_port_create( STB_NET_PORT_TYPE_CLIENT, STB_NET_PORT_PROTO_UDP, &addr, NULL, NULL ); /* port a */
	if(pa == STB_ERROR) {
		printf("error\n");
	}
	printf("port is <%08x>\n",pa);

	stb_net_addrinfo_make(&addr,STB_NET_IP(192,168,2,13),1200);
	pb = stb_net_port_create( STB_NET_PORT_TYPE_CLIENT, STB_NET_PORT_PROTO_UDP, &addr, NULL, NULL ); /* port a */
	if(pa == STB_ERROR) {
		printf("error\n");
	}
	printf("port is <%08x>\n",pb);

	stb_net_addrinfo_make(&addr,STB_NET_IP(192,168,2,13),1300);
	pc = stb_net_port_create( STB_NET_PORT_TYPE_CLIENT, STB_NET_PORT_PROTO_UDP, &addr, NULL, NULL ); /* port a */
	if(pa == STB_ERROR) {
		printf("error\n");
	}
	printf("port is <%08x>\n",pc);

	stb_net_addrinfo_make(&addr,STB_NET_IP(192,168,2,13),1400);
	pd = stb_net_port_create( STB_NET_PORT_PROTO_UDP, &addr, NULL, NULL ); /* port a */
	if(pa == STB_ERROR) {
		printf("error\n");
	}
	printf("port is <%08x>\n",pd);

	/* clean up */
	stb_net_port_proto_select_cleanup(&selg);

	/* port a,b,c */
	if(stb_net_port_proto_select_group(&selg,pa,pb,pc,0)==STB_ERROR) {
		printf("error\n");
		exit(0);
	}

	/* port d */
	if(stb_net_port_proto_select_group(&selg,pd,0)==STB_ERROR) {
		printf("error\n");
		exit(0);
	}

	printf("status = %d %d \n",selg.num,selg.maxfd);
	local_print_select_group(&selg);

	/* deleting pb */
	if(stb_net_port_proto_select_delete(&selg,pb)==STB_ERROR) {
		printf("error\n");
		exit(0);
	}

	printf("and status = %d %d \n",selg.num,selg.maxfd);
	local_print_select_group(&selg);

}

