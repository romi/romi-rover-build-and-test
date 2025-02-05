#!/usr/bin/env bash

# Network Hotspot Setup Script

# This script configures network interfaces and routing tables to set up a device as a Wi-Fi hotspot.
# It is designed to automate the process of configuring a wireless hotspot, making it useful for deploying quick, standalone wireless networks.

# Key Features:
# - Configures wireless and Ethernet interfaces for hotspot functionality.
# - Automatically installs required dependencies (`hostapd`, `dnsmasq`, etc.).
# - Sets up IP routing, static IPs, and DHCP configuration.
# - Supports customizable SSID and passwords for the hotspot.
# - Saves backups of existing network configurations before modifications.
# - Ensures IPv4 routing and firewall rules for NAT (Network Address Translation).
# - Can generate a random password for the hotspot if none is provided.
#
# Usage Examples:
# To set up a hotspot with default parameters:
#   sudo ./network-hotspot-setup.sh
#
# To specify custom interfaces and SSID:
#   sudo ./network-hotspot-setup.sh --wlan wlan1 --eth eth1 --ssid "Custom SSID" --pwd "custompassword"
#
# For help:
#   ./network-hotspot-setup.sh -h

# Default network interface names and SSID information. These can be overridden using options.
WLAN=wlan0         # Default wireless local area network (Wi-Fi) interface name
ETH=eth0           # Default Ethernet interface name
SSID2="Romi Rover" # Default SSID for the hotspot
AP_PWD=""          # Default password for hotspot, random if not specified

#WLAN=wlp0s20f3
#ETH=enp40s0

# From https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md
# And https://cdn-learn.adafruit.com/downloads/pdf/setting-up-a-raspberry-pi-as-a-wifi-access-point.pdf

# Prints usage information when -h or an invalid option is provided.
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
    ;;    # Sets custom wireless interface
  --eth)
    shift
    ETH=$1
    ;;   # Sets custom Ethernet interface
  --ssid)
    shift
    SSID2=$1
    ;;   # Sets custom SSID for the hotspot
  --pwd)
    shift
    AP_PWD=$1
    ;;   # Sets custom password; will use default if omitted
  -h | --help)
    usage
    exit
    ;;    # Displays help message and exits
  *)
    usage
    exit 1
    ;;    # Handles invalid arguments
  esac
  shift
done

# Installs required dependencies in a non-interactive environment.
DEBIAN_FRONTEND=noninteractive apt install -y hostapd dnsmasq netfilter-persistent iptables-persistent

# Enables and unmasks `hostapd`, ensuring it can run at startup.
systemctl unmask hostapd
systemctl enable hostapd

# Generates a unique timestamp for backup filenames.
DATE=`date +"%Y%m%d-%H%M%S"`

# Backs up and modifies DHCP configuration to assign a static IP to the wireless interface.
if [ -e /etc/dhcpcd.conf ]; then
    mv /etc/dhcpcd.conf /etc/dhcpcd.conf-$DATE # Backup existing config
fi

cat <<EOF >> /etc/dhcpcd.conf
interface $WLAN
    static ip_address=10.10.10.1/24 # Assign static IP to the wireless interface
    nohook wpa_supplicant          # Disable wpa_supplicant to avoid conflicts
    static domain_name_servers=8.8.8.8 # Use Google's DNS as default
EOF

# Enables IP forwarding for IPv4 traffic.
cat <<EOF >> /etc/sysctl.d/routed-ap.conf
# https://www.raspberrypi.org/documentation/configuration/wireless/access-point-routed.md
# Enable IPv4 packet forwarding
net.ipv4.ip_forward=1
EOF

# Configures iptables for NAT and packet forwarding rules between wireless and Ethernet interfaces.
iptables -t nat -A POSTROUTING -o $ETH -j MASQUERADE # Enable NAT for outgoing Ethernet traffic
iptables -A FORWARD -i $ETH -o $WLAN -m state --state RELATED,ESTABLISHED -j ACCEPT # Forward traffic from Ethernet to Wi-Fi
iptables -A FORWARD -i $WLAN -o $ETH -j ACCEPT # Forward traffic from Wi-Fi to Ethernet

# Saves iptables rules persistently.
netfilter-persistent save

# Temporarily enables IPv4 packet forwarding until next reboot.
echo 1 > /proc/sys/net/ipv4/ip_forward

# Backs up and modifies dnsmasq configuration for DHCP services.
if [ -e /etc/dnsmasq.conf ]; then
    mv /etc/dnsmasq.conf /etc/dnsmasq.conf-$DATE # Backup existing config
fi
cat <<EOF >> /etc/dnsmasq.conf
interface=$WLAN                                     # Listening interface
dhcp-range=10.10.10.2,10.10.10.20,255.255.255.0,24h # DHCP address pool
domain=wlan                                         # Local DNS domain
address=/gw.wlan/10.10.10.1                         # Maps hostname (e.g., gw.wlan) to router IP
dhcp-option=6,8.8.8.8                               # Set DNS server
EOF

# Ensures the Wi-Fi interface is enabled in case it is blocked by rfkill.
rfkill unblock wlan

# Backs up and modifies hostapd configuration for the wireless hotspot.
if [ -e /etc/hostapd/hostapd.conf ]; then
    mv /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.conf-$DATE # Backup existing config
fi

# Generates a random password if none is provided by the user.
if [ -z ${AP_PWD+x} ]; then
  AP_PWD=`echo $RANDOM | md5sum | head -c 8` # Generate an 8-character random password
  echo "Password set to '$AP_PWD'"
fi

# Configures hostapd with SSID, security, and interface details.
cat <<EOF >> /etc/hostapd/hostapd.conf
country_code=FR
interface=$WLAN
ssid2="$SSID2"              # Set the SSID for the network
hw_mode=g                   # Operates in 2.4 GHz band (802.11g)
channel=10                  # Wi-Fi channel
macaddr_acl=0               # No MAC address filtering
auth_algs=1                 # WPA-only authentication
ignore_broadcast_ssid=0     # Broadcast SSID for discovery
wpa=2                       # Use WPA2 security
wpa_passphrase=$AP_PWD      # Set the WPA passphrase
wpa_key_mgmt=WPA-PSK        # Use pre-shared key authentication
wpa_pairwise=TKIP           # WPA encryption
rsn_pairwise=CCMP           # WPA2 encryption
EOF
