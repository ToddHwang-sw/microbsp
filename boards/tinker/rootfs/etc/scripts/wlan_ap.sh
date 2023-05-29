#!/bin/sh

APCONFDIR=/var/tmp/hostapd
PIDDIR=/var/run/hostapd
CFILE=1.conf

DHCPFILE=dhcpd.conf
LFILE=dhcpd.leases

ASK='xcfgcli.sh get '
BRINTF=`$ASK lan/name`
BRCAPTURE=`$ASK lan/capture`
APINTF=`$ASK lan/interfaces/wlan0`
ETHINTF=`$ASK lan/interfaces/eth0`
APSSID=`$ASK lan/ssid`
APPASSWORD=`$ASK lan/password`
APIP=`$ASK lan/ip`
APNET=${APIP%.*}
APNODE=${APIP##*.}
APMASK=`$ASK lan/netmask`

DHCPSTART=`$ASK lan/start`
DHCPEND=`$ASK lan/end`

tcpdump_CMD="tcpdump -i $BRINTF"
tcpdump_PID=`ps -aef | grep -e "$tcpdump_CMD" | head -n 1 | awk '{print $1}'`

add_ap() {
		APINTF=`$ASK lan/interfaces/wlan$1`

		[ "$APINTF" != "0" ] || return 0

		NAPSSID=$APSSID"-$1"

		##
		## AP hostapd ...
		##
	cat > $APCONFDIR/$1.conf  <<-EOF
	interface=$APINTF
	ctrl_interface=$APCONFDIR/ctrl
	bridge=$BRINTF
	driver=nl80211
	ssid=$NAPSSID
	channel=1
	ieee80211d=1
	ieee80211h=1
	ieee80211n=1
	wmm_enabled=1
	country_code=US
	hw_mode=g
	macaddr_acl=0
	auth_algs=3
	ignore_broadcast_ssid=0
	wpa=2
	wpa_passphrase=$APPASSWORD
	wpa_key_mgmt=WPA-PSK
	wpa_pairwise=TKIP
	rsn_pairwise=CCMP
	EOF

		echo "[WLAN] Running AP Suppplicant for $APINTF"
		/usr/local/bin/hostapd -dd -B -P $PIDDIR/$1.pid $APCONFDIR/$1.conf &
		sleep 1

		##
		## NATBYP setting 
		[ ! -f /proc/natbyp ] || echo "dev $APINTF lan" > /proc/natbyp

		echo "[BRIDGE] Bridge adding $APINTF"
		brctl addif $BRINTF $APINTF

		return 1
}

case $1 in
	"start")
		echo "[WLAN] Building up $APCONFDIR/$CFILE"
		[ -d $APCONFDIR ] || mkdir -p $APCONFDIR
		[ -d $PIDDIR ] || mkdir -p $PIDDIR

		echo "[ETH] Ethernet and Bridge"
		ifconfig $ETHINTF up
		brctl addbr $BRINTF
		brctl addif $BRINTF $ETHINTF

		[ ! -f /proc/natbyp ] || echo "dev $ETHINTF lan" > /proc/natbyp

		echo "[AP] Adding APs..."
		cnt=0
		rc=1
		while [ $rc == 1 ]; do 
			add_ap $cnt
			rc=$?
			cnt=`expr $cnt + 1`
		done

		##
		## IPv4 DHCP server ...
		##
		cat > $APCONFDIR/$DHCPFILE <<-EOF

		subnet $APNET.0 netmask $APMASK {
			range $APNET.$DHCPSTART $APNET.$DHCPEND;
			option domain-name "$DOMAIN";
			option domain-name-servers $APNET.$APNODE;
			option routers $APNET.$APNODE;
			option broadcast-address $APNET.255;
		}
		EOF
		ifconfig $BRINTF $APNET.$APNODE netmask $APMASK up
		[ ! -f $APCONFDIR/$LFILE ] || \rm -rf $APCONFDIR/$LFILE
		touch $APCONFDIR/$LFILE

		##
		## tcpdump 
		##
		[ "$BRCAPTURE" = "0" ] || $tcpdump_CMD -c 99999 -w /var/tmp/pcap_$BRINTF.pcap &
		;;

	"stop")
		echo "[WLAN] TCPDUMP down..."
		[ "$BRCAPTURE" = "0" ] || kill -9 $tcpdump_PID

		echo "[BRIDGE] Bridge shutdown..."
		brctl delif $BRINTF $APINTF
		brctl delif $BRINTF $ETHINTF
		brctl delbr $BRINTF

		echo "[ETH] Ethernet"
		ifconfig $ETHINTF down

		echo "[WLAN] Shutting down..."
		pid=`cat $PIDDIR/hostapd.pid`
		kill -9 $pid

		ifconfig $BRINTF down 
		;;

	"*")
		;;
esac
