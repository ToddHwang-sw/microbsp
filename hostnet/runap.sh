#!/bin/sh

##
## hostapd_g.conf - 11G mode
## hostapd.conf   - 11N mode
##
HOSTAP_N_CONF=./hostapd.conf
HOSTAP_G_CONF=./hostapd_g.conf

##
## Hostapd ctrl interface
[ -d /var/tmp/hostapd/ctrl  ] || mkdir -p /var/tmp/hostapd/ctrl
[ -d /var/tmp/hostapd2/ctrl ] || mkdir -p /var/tmp/hostapd2/ctrl

## Fake job
sudo ufw disable
sudo ufw enable 
sudo ufw disable
sudo iptables -t nat -L -v
sleep 1

sudo brctl addbr br0
sudo brctl addif br0 eth0

\rm -rf ./hostap*.log

## N/A AP
sudo hostapd -ddddd -B $HOSTAP_N_CONF > ./hostap_n.log

## G AP
sudo hostapd -ddddd -B $HOSTAP_G_CONF > ./hostap_g.log
sleep 1
sudo ifconfig br0 up
sudo ifconfig br0 10.5.5.1 netmask 255.255.255.0 up

# DNS
sudo systemctl disable systemd-resolved
sudo systemctl stop systemd-resolved

if [ -s /etc/resolv.conf ]; then
    sudo unlink /etc/resolv.conf
    echo nameserver 8.8.8.8 | sudo tee /etc/resolv.conf
fi

sudo systemctl restart dnsmasq

