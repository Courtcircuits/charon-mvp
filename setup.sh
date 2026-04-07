#!/bin/bash
set -e

TUN_IFACE="tun0"
LOCAL_ADDR="10.0.0.1/24"

# Must run as root
if [ "$(id -u)" -ne 0 ]; then
    echo "Run as root" >&2
    exit 1
fi

ip tuntap add dev "$TUN_IFACE" mode tun
ip addr add "$LOCAL_ADDR" dev "$TUN_IFACE"
ip link set "$TUN_IFACE" up

echo "$TUN_IFACE is up with address $LOCAL_ADDR"
