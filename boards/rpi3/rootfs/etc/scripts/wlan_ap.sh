#!/bin/sh

APCONFDIR=/var/tmp/hostapd
PIDDIR=/var/run/hostapd
CFILE=1.conf

DHCPFILE=dhcpd.conf
LFILE=dhcpd.leases
PFILE=dhcpd.pid

. /boot/conf.sys

tcpdump_CMD="tcpdump -i $BRINTERFACE"
tcpdump_PID=`ps -aef | grep -e "$tcpdump_CMD" | head -n 1 | awk '{print $1}'`

case $1 in
	"start")
		[ "$USEAP" = "1" ] || exit 0

		echo "[WLAN] Building up $APCONFDIR/$CFILE"
		[ -d $APCONFDIR ] || mkdir -p $APCONFDIR
		[ -d $PIDDIR ] || mkdir -p $PIDDIR

		##
		## AP hostapd ...
		##
		cat > $APCONFDIR/$CFILE  <<-EOF
		interface=$APINTERFACE
		ctrl_interface=$APCONFDIR/ctrl
		bridge=$BRINTERFACE
		driver=nl80211
		ssid=$APSSID
		channel=1
		ieee80211d=1
		ieee80211h=1
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

		echo "[WLAN] Running AP Suppplicant !!"
		/usr/local/bin/hostapd -dd -B -P $PIDDIR/hostapd.pid $APCONFDIR/$CFILE &
		sleep 1

		echo "[BRIDGE] Bridge creation...."
		brctl addbr $BRINTERFACE
		brctl addif $BRINTERFACE $ETHINTERFACE
		brctl addif $BRINTERFACE $APINTERFACE

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
		ifconfig $BRINTERFACE $APNET.$APNODE netmask $APMASK up
		[ ! -f $APCONFDIR/$LFILE ] || \rm -rf $APCONFDIR/$LFILE
		touch $APCONFDIR/$LFILE
		dhcpd -4 -d -cf $APCONFDIR/$DHCPFILE -lf $APCONFDIR/$LFILE -pf $PIDDIR/$PFILE $BRINTERFACE &
		sleep 1

		##
		## tcpdump 
		##
		[ "$BRCAPTURE" = "0" ] || $tcpdump_CMD -c 99999 -w /var/tmp/pcap_$BRINTERFACE.pcap &
		;;
	"stop")
		echo "[WLAN] TCPDUMP down..."
		[ "$BRCAPTURE" = "0" ] || kill -9 $tcpdump_PID

		echo "[BRIDGE] Bridge shutdown..."
		brctl delif $BRINTERFACE $APINTERFACE
		brctl delif $BRINTERFACE $ETHINTERFACE
		brctl delbr $BRINTERFACE

		echo "[WLAN] Shutting down..."
		pid=`cat $PIDDIR/hostapd.pid`
		kill -9 $pid

		pid=`cat $PIDDIR/$PFILE`
		kill -9 $pid

		ifconfig $BRINTERFACE down 

		;;
	"*")
		;;
esac
