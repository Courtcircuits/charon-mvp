# Charon MVP

This is my wall of shame for the Charon tunnel project. It contains snippets of code that I have written to discover and learn about networking, DTNs, VPNs and other related topics. It is not meant to be a comprehensive guide, but rather a collection of notes and experiments that I have done along the way.

It works using git tags, so you can check out the different stages of the project by using `git checkout <tag>`.

## Table of tags

> Usage example `git checkout virtual-interface`

- `virtual-interface`: A simple tunnel that forwards TCP traffic from a virtual interface to a physical interface using the `ip` cli + a C program that reads from the virtual interface.


# Targetted DX

1. Launch a TCP server using netcat :

```bash
nc -l 12345
```

2. Launch a DTN node for this server : 

```bash
./ud3tn/build/posix.ud3tn --node-id dtn://b.dtn/ --aap-port 4242 --aap2-socket ud3tn-b.aap2.socket --cla "mtcp:*,4224"
```

3. Launch a DTN node for the client :

```bash
./ud3tn/build/posix.ud3tn --node-id dtn://a.dtn/ --aap-port 4242 --aap2-socket ud3tn-a.aap2.socket --cla "mtcp:*,4225"
```


4. Send a message from the client to the server using netcat :

```bash
echo "Good morning Night City!" | nc localhost 4225
```
