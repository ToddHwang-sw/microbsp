#!/bin/sh

OPR=$1
INTF=$2
DHCPV6C=dhclient
SCRIPT_FILE=/etc/dhclient/dhclient-script
DHCPC_V6_CONF=/var/tmp/dhcpc_v6_$INTF.conf
DHCPC_V6_PID=/var/run/dhcpc_v6_$INTF.pid
DHCPC_V6_LEASE=/var/tmp/dhcpc_v6_$INTF.leases

help() {
	echo "Usage: $0 start <intf>"
	echo "       $0 stop  <intf>"
}

dhcpc_start() {
	# DHCPv6 configuration file creation 	
	cat > $DHCPC_V6_CONF <<-EOF
	send dhcp6.rapid-commit;
	request dhcp6.name-servers, dhcp6.domain-search, dhcp6.fqdn;
	EOF
	[ ! -f $DHCPC_V6_PID   ] || rm -f $DHCPC_V6_PID
	[ ! -f $DHCPC_V6_LEASE ] || rm -f $DHCPC_V6_LEASE
	touch $DHCPC_V6_LEASE
	touch $DHCPC_V6_PID
	$DHCPV6C -6 -cf $DHCPC_V6_CONF -sf $SCRIPT_FILE -lf $DHCPC_V6_LEASE -pf $DHCPC_V6_PID $INTF &
}

dhcpc_stop() {
	kill -9 `cat $DHCPC_V6_PID`
	[ ! -f $DHCPC_V6_LEASE ] || rm -f $DHCPC_V6_LEASE
	[ ! -f $DHCPC_V6_CONF  ] || rm -f $DHCPC_V6_CONF
	[ ! -f $DHCPC_V6_PID   ] || rm -f $DHCPC_V6_PID
}

case $OPR in
    start)
        dhcpc_start 
        ;;
    stop) 
        dhcpc_stop 
        ;;
    *) # Display usage if no parameters given
        help
esac

