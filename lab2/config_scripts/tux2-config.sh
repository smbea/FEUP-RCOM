#!/bin/bash

ifconfig eth0 up
ifconfig eth0 172.16.51.1/24
route add -net 172.16.50.0/24 gw 172.1.51.253 eth0
route add default gw 172.16.51.254