#!/bin/sh

. /boot/conf.sys

OPR=$1
LAN=$BRINTERFACE

PREFIX_LEN=64
GPREFIX_LEN=128

DNSCONF=/var/tmp/dnsmasq.conf
DNSPIDFILE=/var/run/dnsmasq.pid

RADVD_CONF=/var/tmp/radvd_$LAN.conf
RADVD_PID=/var/run/radvd_$LAN.pid
PREFIXFILE=/var/tmp/ipv6_$LAN.prefix
ADDRFILE=/var/tmp/ipv6gaddr_$LAN
DHCPD_V6_CONF=/var/tmp/dhcpd_v6_$LAN.conf
DHCPD_V6_PID=/var/run/dhcpd_v6_$LAN.pid
DHCPD_V6_LEASE=/var/tmp/dhcpd_v6_$LAN.leases
NPDCONF=/var/tmp/npd6_$LAN.conf

GOOGLEDNS1=2001:4860:4860::8888
GOOGLEDNS2=2001:4860:4860::8844

radvd_start() {

	if [ -f $RADVD_CONF ]; then 
		rm -f $RADVD_CONF
	fi

	if [ "$USE_NAT6" = "1" ]; then
                IPV6NS="$IPV6_ADDR"
        else
		IPV6NS=$(echo `cat /etc/resolv.conf | grep ":" | sed -e 's/nameserver //g'`)
        fi

	cat > $RADVD_CONF <<-EOF
	interface $LAN {
		AdvSendAdvert on;
		MinRtrAdvInterval 3;
		MaxRtrAdvInterval 10;
		AdvDefaultPreference high;
		AdvDefaultLifetime 8999;
		AdvReachableTime 30000;
		AdvRetransTimer 1000;

		prefix $PREFIX_ADDR::/$PREFIX_LEN {
			AdvOnLink on;
			AdvAutonomous on;
			AdvRouterAddr off;
			AdvValidLifetime 4294967295;
			AdvPreferredLifetime 4294967295;
		};

		RDNSS $IPV6NS {
		};
	}; 
	EOF

	# update prefix file
	echo $PREFIX_ADDR > $PREFIXFILE

	# global address setting ~
	[ "$IPV6INFO" = "" ] || ifconfig $LAN add $IPV6_ADDR/$GPREFIX_LEN
	echo $IPV6_ADDR > $ADDRFILE

	#
	# Neighbor proxy
	[ ! -f $NPDCONF ] || rm $NPDCONF
	touch $NPDCONF
	cat > $NPDCONF <<-EOF

	prefix=$PREFIX_ADDR:
	interface=$LAN
	EOF
	[ "$IPV6INFO" = "" ] || npd6 -c $NPDCONF

	# Run radvd (2 process)
	[ "$IPV6INFO" = "" ] || ( radvd -C $RADVD_CONF -p $RADVD_PID;  sleep 3 )

	# DHCPv6
	BR0_IPV6_ADDR=`ifconfig br0 | grep "Scope:Link" | awk {'print $3'} | cut -d "/" -f 1`
	cat > $DHCPD_V6_CONF <<-EOF
	authoritative;
	subnet6 $PREFIX_ADDR::/$PREFIX_LEN {
		option dhcp6.name-servers $BR0_IPV6_ADDR;
	}
	EOF
	rm -f $DHCPD_V6_LEASE
	touch $DHCPD_V6_LEASE
	[ "$IPV6INFO" = "" ] || ( dhcpd -6 -cf $DHCPD_V6_CONF -lf $DHCPD_V6_LEASE -pf $DHCPD_V6_PID $LAN ; sleep 1 )

	echo "[DNS] Masquerading start ..."
	echo "$APNET.$APNODE  $DOMAIN" > $DNSCONF
	[ "$IPV6INFO" = "" ] || echo "$IPV6_PREFIX$IPV6_ID  $DOMAIN" >> $DNSCONF
	dnsmasq -a $APNET.$APNODE -i $BRINTERFACE -H $DNSCONF -r /etc/resolv.conf -u root -x $DNSPIDFILE
	sleep 1
}

radvd_stop() {

	## Killing DHCPD
	kill -9 `cat $DHCPD_V6_PID`
	[ ! -f $DHCPD_V6_CONF ] || rm -f $DHCPD_V6_CONF

	## Killing RADVD
	kill -9 `cat $RADVD_PID`
	[ ! -f $RADVD_CONF ] || rm -f $RADVD_CONF

	## Killing NPD6
	kill -9 `ps - | grep $NPD6 | grep $LAN | awk '{print $1}'`
	[ ! -f $NPDCONF ] || rm -f $NPDCONF

	## Deleting interface address & file ..
	if [ -f $ADDRFILE ]; then 
		GADDR=$(cat $ADDRFILE)
		ip -6 route del $PREFIX_ADDR:/$PREFIX_LEN dev $LAN
		ifconfig $LAN del $GADDR/$GPREFIX_LEN
		rm $ADDRFILE
	fi
}

IPV6INFO=`ifconfig $STAINTERFACE | grep -e "Scope:Global" | awk '{print $3}'`
if [ "$IPV6INFO" != "" ]; then
        if [ "$USE_NAT6" = "0" ]; then
                IPV6_PREFIX=${IPV6INFO%::*}"::"
        fi
fi

IPV6_ADDR=$IPV6_PREFIX$IPV6_ID
TEMP_VALUE=$(expr $PREFIX_LEN / 8 / 2)
PREFIX_ADDR_STR_LEN=$(expr $TEMP_VALUE \* 4 + $TEMP_VALUE - 1)
PREFIX_ADDR=${IPV6_ADDR:0:$PREFIX_ADDR_STR_LEN}

case $OPR in
        start)
                radvd_start 
                ;;
        stop) 
                radvd_stop 
                ;;
        *) 
		;;
esac

