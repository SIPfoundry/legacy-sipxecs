#!/bin/sh
echo $1
echo $2
cat tester-config.xml.in | sed -e "s/tester-config.tester-ip-address/$1/" > tester-config.xml
cat $2/sipxbridge.xml.in | sed -e "s/tester-config.tester-ip-address/$1/" > $2/sipxbridge.xml
cat $2/test-maps.xml.in | sed -e "s/tester-config.tester-ip-address/$1/" > $2/test-maps.xml
