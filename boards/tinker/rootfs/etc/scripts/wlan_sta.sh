#!/bin/sh

WPACONFDIR=/var/tmp/wpa_supplicant
PIDDIR=/var/run/wpa_supplicant
CFILE=1.conf

ASK='xcfgcli.sh get '
STAINT=`$ASK wan/name`
USESTA=`$ASK wan/mode`
STACAPTURE=`$ASK wan/capture`
STASSID=`$ASK wan/ssid`
STAPASSWORD=`$ASK wan/password`

case $1 in
	"start")
		[ "$USESTA" != "none" ] || exit 0

		ifconfig $STAINT up

		##
		## tcpdump 
		##
		[ "$STACAPTURE" = "0" ] || \
			tcpdump -i $STAINT -c 99999 -w /var/tmp/pcap_$STAINT.pcap &

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

		## eth0 MAC address
		ETHMAC=`$ASK wan/mac`
		if [ "$ETHMAC" = "random" ]; then
			ETHMAC=`tr -dc A-F0-9 < /dev/urandom | head -c 10 | sed -r 's/(..)/\1:/g;s/:$//;s/^/02:/'`
		fi
		ifconfig $STAINT hw ether $ETHMAC

		echo "[WLAN] Running WPA Suppplicant !!"
		[ -d $PIDDIR ] || mkdir -p $PIDDIR
		wpa_supplicant -B -D wext -i $STAINT -c $WPACONFDIR/$CFILE -P $PIDDIR/wpa.pid
		cnt=0
		while [ $cnt -lt 8 ];  do
			echo "Waiting Connect .. $cnt"
			sleep 1
			cnt=`expr $cnt + 1`
		done

		##
		## NATBYP setting 
		[ ! -f /proc/natbyp ] || echo "dev $STAINT wan" > /proc/natbyp

		NETMODE=`$ASK wan/mode`
		if [ "$NETMODE" = "dhcp" ]; then
			echo "IPv4 DHCP ..."
			/etc/scripts/dhcpc.sh start $STAINT
		else
			IP=`$ASK wan/ip`
			NETMASK=`$ASK wan/netmask`
			ifconfig $STAINT $IP netmask $NETMASK up
			route add -net 0.0.0.0 dev $STAINT
		fi

		#
		# IPv4 
		#

		## Waiting for IPv4 address seconds ...
		cnt=0
		while [ $cnt -lt 5 ]; do
			ipnum=`ifconfig $STAINT | grep "inet addr" | wc -l`
			if [ "$ipnum" = "0" ]; then
				echo "Waiting IP .. $cnt"
				sleep 1
			else
				ipaddr=`ifconfig $STAINT | grep "inet addr" | awk '{print $2}'`
				ipaddr=${ipaddr#addr:}
				echo "IP address '$ipaddr' configured."
				break
			fi
			cnt=`expr $cnt + 1`
		done
		;;
	"stop")
		echo "[WLAN] Shutting down..."
		/etc/scripts/dhcpc.sh stop $STAINT

		pid=`cat $PIDDIR/wpa.pid`
		kill -9 $pid
		pid=`cat $PIDDIR/udhcpc.pid`
		kill -9 $pid
		ifconfig $STAINT down 

		echo "[WLAN] TCPDUMP down..."
		[ "$STACAPTURE" = "0" ] || killall tcpdump
		;;
	"*")
		;;
esac
