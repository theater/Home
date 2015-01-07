#!/bin/bash
echo > /hab/openhab/logs/openhab.log
echo > /hab/openhab/logs/events.log
rm /hab/openhab/logs/*request.log
rm /hab/openhab/logs/*.zip
rm /var/log/*.gz
rm /var/log/*.1*

echo > /var/log/syslog
echo > /var/log/messages
echo > /var/log/lastlog
echo > /var/log/debug
echo > /var/log/daemon.log
echo > /var/log/wtmp
echo > /var/log/auth.log
echo > /var/log/kern.log
echo > /var/log/faillog
echo > /var/log/bootstrap.log
