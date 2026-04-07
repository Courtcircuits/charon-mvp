# Setup

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
