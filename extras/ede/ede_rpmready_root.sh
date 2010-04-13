#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# ede_rpmready_root.sh
#
# This gets a system ready to have the sipXecs RPMs installed.

if [ "`whoami`" != root ]; then
  echo "You must be root in order to run this script."
  exit 1
fi

# Disable SELinux (could be done from a GUI install, but just to be sure....)
if [ ! -e /etc/selinux/config_ORIG ] 
then
  cp /etc/selinux/config /etc/selinux/config_ORIG
fi
echo SELINUX=disabled > /etc/selinux/config # This prevents it from starting following reboots.
echo 0 >/selinux/enforce # This stops it right now, without a reboot.

# Disable the Firewall (could be done from a GUI install, but just to be sure....)
/sbin/service iptables stop
/sbin/chkconfig iptables off

