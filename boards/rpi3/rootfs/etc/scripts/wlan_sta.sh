#!/bin/sh

WPACONFDIR=/var/tmp/wpa_supplicant
PIDDIR=/var/run/wpa_supplicant
CFILE=1.conf

. /boot/conf.sys

tcpdump_CMD="tcpdump -i $STAINTERFACE"
tcpdump_PID=`ps -aef | grep -e $tcpdump_CMD | head -n 1 | awk '{print $1}'`

case $1 in
	"start")
		[ "$USESTA" = "1" ] || exit 0

		ifconfig $STAINTERFACE up

		##
		## tcpdump 
		##
		[ "$STACAPTURE" = "0" ] || $tcpdump_CMD -c 99999 -w /var/tmp/pcap_$STAINTERFACE.pcap &

		echo "[WLAN] Building up $WPACONFDIR/$CFILE"
		[ -d $WPACONFDIR ] || mkdir -p $WPACONFDIR

		cat > $WPACONFDIR/$CFILE <<-EOF
		ctrl_interface=$WPACONFDIR/ctrl
		update_config=1
		eapol_version=1
		ap_scan=1
		fast_reauth=1
		country=US
		network={
			ssid="$STASSID"
			psk="$STAPASSWORD"
			scan_ssid=1
		}
		EOF

		echo "[WLAN] Running WPA Suppplicant !!"
		[ -d $PIDDIR ] || mkdir -p $PIDDIR
		wpa_supplicant -B -D wext -i $STAINTERFACE -c $WPACONFDIR/$CFILE -P $PIDDIR/wpa.pid
		cnt=0
		while [ $cnt -lt 3 ];  do
			echo "Waiting Connect .. $cnt"
			sleep 1
			cnt=`expr $cnt + 1`
		done

		#
		# IPv4 
		#
		echo "IPv4 DHCP ..."
		/etc/scripts/dhcpc.sh start $STAINTERFACE

		## Waiting for IPv4 address seconds ...
		cnt=0
		while [ $cnt -lt 5 ]; do
			ipnum=`ifconfig $STAINTERFACE | grep "inet addr" | wc -l`
			if [ "$ipnum" = "0" ]; then
				echo "Waiting IP .. $cnt"
				sleep 1
			else
				ipaddr=`ifconfig $STAINTERFACE | grep "inet addr" | awk '{print $2}'`
				ipaddr=${ipaddr#addr:}
				echo "IP address '$ipaddr' configured."
				break
			fi
			cnt=`expr $cnt + 1`
		done

		;;
	"stop")
		echo "[WLAN] Shutting down..."
		/etc/scripts/dhcpc.sh   stop $STAINTERFACE

		pid=`cat $PIDDIR/wpa.pid`
		kill -9 $pid
		pid=`cat $PIDDIR/udhcpc.pid`
		kill -9 $pid
		ifconfig $STAINTERFACE down 

		echo "[WLAN] TCPDUMP down..."
		[ "$STACAPTURE" = "0" ] || kill -9 $tcpdump_PID
		;;
	"*")
		;;
esac
