#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# ede_rpminstall_root.sh
# 
# This retrieves sipXecs RPMs from a remote EDE system, and installs them via a local
# yum repository.  (yum will pull in dependency packages from the standard 
# repositories.)  Note that the EDE system must build the RPMs (all of them.)
# i.e. with:
#     > ./ede_build_devuser.sh -r -d

if [ "`whoami`" != root ]; then
  echo "You must be root in order to run this script."
  exit 1
fi

if [ $# -lt 1 -o $# -gt 2 ]; then
  echo "Usage: ${0} <EDE RPM build machine> [WORKING dir]"
  exit 2
fi

EDE_SERVER=$1

WORKING_DIR="WORKING"
if [ $# -eq 2 ]; then
   WORKING_DIR=$2
fi

# Get RPMs copied over from the EDE RPM build machine.
FULL_PATH_EDE_LOGS="."
FULL_PATH_DIST=`pwd`
rm -rf RPM
mkdir RPM
pushd RPM
scp "sipxchange@$EDE_SERVER:./$WORKING_DIR/DIST/RPM/*.rpm" .
if [ $? != 0 ]; then
   echo "ERROR: Failed to scp "sipxchange@$EDE_SERVER:./$WORKING_DIR/DIST/RPM/*.rpm"." >&2
   exit 3
fi
popd
yum -y install createrepo
if [ $? != 0 ]; then
   echo "ERROR: Failed to yum -y install createrepo." >&2
   exit 4
fi
echo [sipxecs-local] > $FULL_PATH_EDE_LOGS/sipxecs-local.repo
echo name=sipXecs dependencies local >> $FULL_PATH_EDE_LOGS/sipxecs-local.repo
echo baseurl=file://$FULL_PATH_DIST/RPM >> $FULL_PATH_EDE_LOGS/sipxecs-local.repo
echo enabled=1 >> $FULL_PATH_EDE_LOGS/sipxecs-local.repo
echo gpgcheck=0 >> $FULL_PATH_EDE_LOGS/sipxecs-local.repo
cp $FULL_PATH_EDE_LOGS/sipxecs-local.repo /etc/yum.repos.d/
createrepo $FULL_PATH_DIST/RPM
if [ $? != 0 ]; then
   echo "ERROR: Failed to createrepo." >&2
   exit 5
fi
yum -y install sipxecs
if [ $? != 0 ]; then
   echo "ERROR: Failed to yum install sipxecs." >&2
   exit 6
fi
sipxecs-setup
if [ $? != 0 ]; then
   echo "ERROR: Failed to sipxecs-setup." >&2
   exit 7
fi
