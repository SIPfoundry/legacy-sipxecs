#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# sipxinterop-revert.in.sh

if [ -d ~/XX-6078-revert ]; then
   echo "Reverting with ~/XX-6078-revert..."
   cp ~/XX-6078-revert/process.d/* @SIPX_DATADIR@/process.d
   cp ~/XX-6078-revert/sipdb/* @SIPX_DBDIR@
   cp ~/XX-6078-revert/homedir/*.xml @SIPX_CONFDIR@
   cp ~/XX-6078-revert/homedir/*.conf @SIPX_CONFDIR@
   cp ~/XX-6078-revert/homedir/*-config @SIPX_CONFDIR@
   cp ~/XX-6078-revert/http_rootdir/* @HTTP_ROOTDIR@
   rm -rf ~/XX-6078-revert
else
   echo "ERROR: No ~/XX-6078-revert directory found!" >&2
   exit 1
fi

@SERVICEDIR@/sipxecs restart
if [ $? != 0 ]; then
   echo "ERROR: Failed to restart sipXecs!" >&2
   exit 2
fi
