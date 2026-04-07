#!/bin/bash
set -e

TUN_IFACE="tun0"
# The local address for the TUN interface. This should be in a private IP range and not conflict with any existing network.
LOCAL_ADDR="10.0.0.1/24"

# Must run as root
if [ "$(id -u)" -ne 0 ]; then
    echo "Run as root" >&2
    exit 1
fi

# Create the TUN interface
ip tuntap add dev "$TUN_IFACE" mode tun
# Assign an IP address to the TUN interface
ip addr add "$LOCAL_ADDR" dev "$TUN_IFACE"
# Bring the TUN interface up
ip link set "$TUN_IFACE" up

echo "$TUN_IFACE is up with address $LOCAL_ADDR"

# Quick note : the setup is not exactly the same as wg, since wg provides a kernel module that adds a dedicated "wireguard" interface type.
# but under the hood, wg is also using a TUN interface to handle the encrypted traffic, so the setup is quite similar : https://git.zx2c4.com/wireguard-linux/tree/Documentation/networking/tuntap.rst
