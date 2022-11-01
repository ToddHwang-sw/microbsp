#!/bin/sh

OPR=$1
INTF=$2
DHCPV4C=dhclient
SCRIPT_FILE=/etc/dhclient/dhclient-script
DHCPC_V4_CONF=/var/tmp/dhcpc_v4_$INTF.conf
DHCPC_V4_PID=/var/run/dhcpc_v4_$INTF.pid
DHCPC_V4_LEASE=/var/tmp/dhcpc_v4_$INTF.leases

help() {
	echo "Usage: $0 start <intf>"
	echo "       $0 stop  <intf>"
}

dhcpc_start() {
	# DHCPv6 configuration file creation 	
	cat > $DHCPC_V4_CONF <<-EOF
	send dhcp-lease-time 120;
	prepend domain-name-servers 127.0.0.1;
	request subnet-mask, broadcast-address, routers, domain-name-servers;
	timeout 60;
	reboot 10;
	select-timeout 5;
	initial-interval 2;
	EOF

	[ ! -f $DHCPC_V4_LEASE ] || rm -f $DHCPC_V4_LEASE
	touch $DHCPC_V4_LEASE
	touch $DHCPC_V4_PID
	$DHCPV4C -4 -cf $DHCPC_V4_CONF -sf $SCRIPT_FILE -lf $DHCPC_V4_LEASE -pf $DHCPC_V4_PID $INTF &
}

dhcpc_stop() {
	kill -9 `cat $DHCPC_V4_PID`
	[ ! -f $DHCPC_V4_LEASE ] || rm -f $DHCPC_V4_LEASE
	[ ! -f $DHCPC_V4_CONF  ] || rm -f $DHCPC_V4_CONF
	[ ! -f $DHCPC_V4_PID   ] || rm -f $DHCPC_V4_PID
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

