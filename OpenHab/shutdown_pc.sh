#!/bin/sh
# Shutdown script

ipaddress=$1
user=$2
password=$3

net rpc shutdown --comment "Shutdown request from openHAB..." --standby -I $ipaddress -U $user%$password
