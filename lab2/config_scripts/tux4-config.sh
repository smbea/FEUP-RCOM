#!/bin/bash

ifconfig eth0 up
ifconfig eth0 172.16.50.254/24

ifconfig eth1 up
ifconfig eth1 172.16.51.253/24

echo 1 >/proc/sys/net/ipv4/ip_forward

route add default gw 172.16.51.254