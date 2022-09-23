#!/usr/bin/env bash

LAN=wlan0
ETH=eth0
#WLAN=wlp0s20f3
#ETH=enp40s0

# From https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md
# And https://cdn-learn.adafruit.com/downloads/pdf/setting-up-a-raspberry-pi-as-a-wifi-access-point.pdf

DEBIAN_FRONTEND=noninteractive apt install -y hostapd dnsmasq netfilter-persistent iptables-persistent

systemctl unmask hostapd
systemctl enable hostapd

DATE=`date +"%Y%m%d-%H%M%S"`

if [ -e /etc/dhcpcd.conf ];
then
    mv /etc/dhcpcd.conf /etc/dhcpcd.conf-$DATE
fi

cat<<EOF >> /etc/dhcpcd.conf
interface $WLAN
    static ip_address=10.10.10.1/24
    nohook wpa_supplicant
EOF

cat<<EOF >> /etc/sysctl.d/routed-ap.conf
# https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md
# Enable IPv4 routing
net.ipv4.ip_forward=1
EOF


iptables -t nat -A POSTROUTING -o $ETH -j MASQUERADE
iptables -A FORWARD -i $ETH -o $WLAN -m state --state RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i $WLAN -o $ETH -j ACCEPT

iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE
iptables -A FORWARD -i eth1 -o $WLAN -m state --state RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i $WLAN -o eth1 -j ACCEPT

netfilter-persistent save

echo 1 > /proc/sys/net/ipv4/ip_forward


if [ -e /etc/dnsmasq.conf ];
then
    mv /etc/dnsmasq.conf /etc/dnsmasq.conf-$DATE
fi

cat<<EOF >> /etc/dnsmasq.conf
interface=$WLAN # Listening interface
dhcp-range=10.10.10.2,10.10.10.20,255.255.255.0,24h
                # Pool of IP addresses served via DHCP
domain=wlan     # Local wireless DNS domain
address=/gw.wlan/10.10.10.1
                # Alias for this router
EOF

rfkill unblock wlan


if [ -e /etc/hostapd/hostapd.conf ];
then
    mv /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.conf-$DATE
fi

# Environment variable defining SSID name:
if [[ -z "${SSID2}" ]]; then
  SSID2="Romi Rover"
fi
# Environment variable defining password:
if [[ -z "${AP_PWD}" ]]; then
  AP_PWD="p2pfoodlab"
fi

cat<<EOF >> /etc/hostapd/hostapd.conf
country_code=FR
interface=$WLAN
ssid2=$SSID2
hw_mode=g
channel=10
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=$AP_PWD
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF
