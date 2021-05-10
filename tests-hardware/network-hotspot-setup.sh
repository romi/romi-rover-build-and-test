#!/usr/bin/env bash


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
interface wlan0
    static ip_address=10.10.10.1/24
    nohook wpa_supplicant
EOF

cat<<EOF >> /etc/sysctl.d/routed-ap.conf
# https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md
# Enable IPv4 routing
net.ipv4.ip_forward=1
EOF


iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
iptables -A FORWARD -i eth0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i wlan0 -o eth0 -j ACCEPT

iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE
iptables -A FORWARD -i eth1 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i wlan0 -o eth1 -j ACCEPT

netfilter-persistent save

echo 1 > /proc/sys/net/ipv4/ip_forward


if [ -e /etc/dnsmasq.conf ];
then
    mv /etc/dnsmasq.conf /etc/dnsmasq.conf-$DATE
fi

cat<<EOF >> /etc/dnsmasq.conf
interface=wlan0 # Listening interface
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

cat<<EOF >> /etc/hostapd/hostapd.conf
country_code=FR
interface=wlan0
ssid2="Romi Rover"
hw_mode=g
channel=10
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=p2pfoodlab
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
EOF
