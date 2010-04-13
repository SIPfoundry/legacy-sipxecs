#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# ede_fedora_staticip_root.sh
#
# See http://wiki.sipfoundry.org/display/xecsdev/Express+Development+Environment+Setup for instructions.

# Changes a Fedora 10/11 or CentOS 5 system from 'NetworkManager' service DHCP to 
# 'network' service static IP, using the specified IP Address.  The network connection 
# must be functioning in order for this to work.

# The files we need to get right for the network service to work.
ETHX=eth0
FILE_ETHX=/etc/sysconfig/network-scripts/ifcfg-$ETHX
FILE_NETWORK=/etc/sysconfig/network
FILE_HOSTS=/etc/hosts
FILE_RESOLV_CONF=/etc/resolv.conf
COPY_RESOLV_CONF=/tmp/COPY_resolv.conf

if [ "`whoami`" != root ]; then
  echo "You must be root in order to run this script."
  exit 1
fi

if [ $# -lt 1 -o $# -gt 2 ]; then
  echo "Usage: ${0} <IP Address> [FQDN]"
  exit 2
fi

yum -y install ruby &> /dev/null

if [ $# -eq 2 ]; then
   FQDN=$2
else
   FQDN=`dig -x $1 | grep -b1 "ANSWER SECTION" | tail -1 | cut -f3 | sed -e "s/.$//g"`
   if [ "$FQDN" == "" ]
   then
     echo "Failed to determine new FQDN."
     exit 3
   fi
fi
HOSTNAME=`echo $FQDN | cut -d. -f1`
if [ "$HOSTNAME" == "" ]; then
  echo "Failed to determine new hostname."
  exit 4
fi

GATEWAY=`/sbin/route -v -n | grep " UG " | ruby -e 'puts STDIN.readline.split[1]'`
if [ "$GATEWAY" == "" ]; then
  echo "Failed to determine existing gateway address."
  exit 5
fi

SUBNET=`/sbin/route -v | head -n3 | tail -n1 | ruby -e 'puts STDIN.readline.split[2]'`
if [ "$SUBNET" == "" ]; then
  echo "Failed to determine existing subnet mask."
  exit 6
fi

echo "   IP Address : $1"
echo "   Subnet Mask: $SUBNET"
echo "   FQDN       : $FQDN"
echo "   Hostname   : $HOSTNAME"
echo "   Gateway    : $GATEWAY"

# Copy the existing /etc/resolv.conf *before* disabling NetworkManager.
rm -rf $COPY_RESOLV_CONF
cp $FILE_RESOLV_CONF $COPY_RESOLV_CONF

# Stop and disable the NetworkManager service, which may not be running.
service NetworkManager stop
chkconfig NetworkManager off

# Stop the network service too.  (Allows this script to work on CentOS 5 DHCP systems.)
service network stop

# /etc/sysconfig/network-scripts/ifcfg-ethX
echo "DEVICE=$ETHX" > $FILE_ETHX
echo "ONBOOT=yes" >> $FILE_ETHX
echo "BOOTPROTO=static" >> $FILE_ETHX
echo "IPADDR=$1" >> $FILE_ETHX
echo "NETMASK=$SUBNET" >> $FILE_ETHX
echo "GATEWAY=$GATEWAY" >> $FILE_ETHX
# Could have put the following here, instead of resolv.conf: SEARCH, DNS1, DNS2, DOMAIN.

# /etc/hosts
echo -e "127.0.0.1\t\tlocalhost.localdomain localhost" > $FILE_HOSTS
echo -e "::1\t\tlocalhost6.localdomain6 localhost6" >> $FILE_HOSTS
echo -e "$1\t\t$FQDN $HOSTNAME" >> $FILE_HOSTS

# /etc/sysconfig/network
sed -i -e "s/HOSTNAME/# previous HOSTNAME/g" $FILE_NETWORK
sed -i -e "s/GATEWAY/# previous GATEWAY/g" $FILE_NETWORK
echo "HOSTNAME=$FQDN" >> $FILE_NETWORK

# hostname
hostname $FQDN

# Use the old /etc/resolv.conf, it will do nicely.
echo "# Cloned by $0 for re-use with the 'network' service." >> $COPY_RESOLV_CONF
cp $COPY_RESOLV_CONF $FILE_RESOLV_CONF

# Enable and start the network service.
chkconfig network on
service network start

# Show off the fruits of our labours.
ruby -e '1.step(20,1){|i| print "."; $stdout.flush; sleep 1}'
ping -c 1 $FQDN 
ping -c 1 nortel.com


