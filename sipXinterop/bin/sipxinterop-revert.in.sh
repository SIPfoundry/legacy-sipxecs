#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# sipxinterop-revert.in.sh

if [ -d ~/interop-revert ]; then
   echo "Reverting with ~/interop-revert..."
   cp ~/interop-revert/process.d/* @SIPX_DATADIR@/process.d
   cp ~/interop-revert/sipdb/* @SIPX_DBDIR@
   cp ~/interop-revert/homedir/*.xml @SIPX_CONFDIR@
   cp ~/interop-revert/homedir/*.conf @SIPX_CONFDIR@
   cp ~/interop-revert/homedir/*-config @SIPX_CONFDIR@
   cp ~/interop-revert/http_rootdir/* @HTTP_ROOTDIR@
   rm -rf ~/interop-revert
else
   echo "ERROR: No ~/interop-revert directory found!" >&2
   exit 1
fi

@SERVICEDIR@/sipxecs restart
if [ $? != 0 ]; then
   echo "ERROR: Failed to restart sipXecs!" >&2
   exit 2
fi
