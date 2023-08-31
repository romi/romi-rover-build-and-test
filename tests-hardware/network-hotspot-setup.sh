#!/usr/bin/env bash

WLAN=wlan0
ETH=eth0
SSID2="Romi Rover"
AP_PWD=""

#WLAN=wlp0s20f3
#ETH=enp40s0

# From https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md
# And https://cdn-learn.adafruit.com/downloads/pdf/setting-up-a-raspberry-pi-as-a-wifi-access-point.pdf

usage() {
  echo "USAGE:"
  echo "  sudo ./network-hotspot-setup.sh [OPTIONS]
    "

  echo "DESCRIPTION:"
  echo "  Configure the network interfaces and routing tables to enable the controller to act as a hotspot.
  Has to be run as root as it will install the required dependencies.
  "

  echo "OPTIONS:"
  echo "  --wlan
    The name of the wireless lan interface, defaults to '$WLAN'."
  echo "  --eth
    The name of the ethernet interface, defaults to '$ETH'."
  echo "  --ssid
    The name of the SSID to configure, defaults to '$SSID2'"
  echo "  --pwd
    The SSID password to configure, defaults to a set of 8 random alphanumeric values (printed to the terminal)."
  # General options:
  echo "  -h, --help
    Output a usage message and exit."
}

while [ "$1" != "" ]; do
  case $1 in
  --wlan)
    shift
    WLAN=$1
    ;;
  --eth)
    shift
    ETH=$1
    ;;
  --ssid)
    shift
    SSID2=$1
    ;;
  --pwd)
    shift
    AP_PWD=$1
    ;;
  -h | --help)
    usage
    exit
    ;;
  *)
    usage
    exit 1
    ;;
  esac
  shift
done


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
dhcp-option=6,8.8.8.8
EOF

rfkill unblock wlan


if [ -e /etc/hostapd/hostapd.conf ];
then
    mv /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.conf-$DATE
fi

# Variable defining SSID password:
if [ -z ${AP_PWD+x} ]; then
  AP_PWD=`echo $RANDOM | md5sum | head -c 8`
  echo "Password set to '$AP_PWD'"
fi

cat<<EOF >> /etc/hostapd/hostapd.conf
country_code=FR
interface=$WLAN
ssid2="$SSID2"
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
