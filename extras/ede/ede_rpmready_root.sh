#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# This gets a system ready to have the sipXecs RPMs installed.
#   - ede_rpmready_root.sh

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

# Required package.  (sipXecs RPMs do not have this dependency?)
yum -y install bind
if [ $? != 0 ]; then
   echo "ERROR: Failed to yum install bind." >&2
   exit 2
fi


# Get RPMs copied over from a build EDE.
#   BUILD_SERVER=?????
#   FULL_PATH_EDE_LOGS="."
#   FULL_PATH_DIST=`pwd`
#   mkdir RPM
#   pushd RPM
#   scp "sipxchange@$BUILD_SERVER:./WORKING/DIST/RPM/*.rpm" .
#      # - password...
#   popd
#   yum -y install createrepo
#   echo [sipxecs-local] > $FULL_PATH_EDE_LOGS/sipxecs-local.repo
#   echo name=sipXecs dependencies local >> $FULL_PATH_EDE_LOGS/sipxecs-local.repo
#   echo baseurl=file://$FULL_PATH_DIST/RPM >> $FULL_PATH_EDE_LOGS/sipxecs-local.repo
#   echo enabled=1 >> $FULL_PATH_EDE_LOGS/sipxecs-local.repo
#   echo gpgcheck=0 >> $FULL_PATH_EDE_LOGS/sipxecs-local.repo
#   cp $FULL_PATH_EDE_LOGS/sipxecs-local.repo /etc/yum.repos.d/
#   createrepo $FULL_PATH_DIST/RPM
#   yum install sipxecs
# then run sipxecs-setup
