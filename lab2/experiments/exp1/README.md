# Configure an IP Network

## Instructions

1. First we connect the both tux51 and tux54 via ethernet 0 ports.
2. Then we setup the IP addresses for tux51 and tux54.

```bash
#Tux 51
ifconfig eth0 up
ifconfig eth0 172.16.50.1/24
#Tux51 now has an IP address 172.16.50.1. The network address uses 24 bits, the remaning 8 bits are used for specificing the host. The subnet mask will look like 255.255.255.0

# Tux54
ifconfig eth0 up
ifconfig eth0 172.16.50.254/24
```

3. 

## Questions

1. What are the ARP packets and what are they used for?

ARP (Address Resolution Protocol) packets ...

2.  

3. What packets does the ping command generate?
