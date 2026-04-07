#!/bin/bash
# setup-dev-env.sh

# Create two namespaces simulating "client" and "server"
ip netns add client
ip netns add server

# Create a virtual ethernet pair (veth) — like a virtual cable between them
ip link add veth-client type veth peer name veth-server

# Assign each end to its namespace
ip link set veth-client netns client
ip link set veth-server netns server

# Assign IPs to the veth interfaces (the "physical" underlay network)
ip netns exec client ip addr add 192.168.100.1/24 dev veth-client
ip netns exec server ip addr add 192.168.100.2/24 dev veth-server

# Bring interfaces up
ip netns exec client ip link set veth-client up
ip netns exec client ip link set lo up
ip netns exec server ip link set veth-server up
ip netns exec server ip link set lo up

echo "✓ Dev environment ready"
echo "  Client underlay IP : 192.168.100.1"
echo "  Server underlay IP : 192.168.100.2"
