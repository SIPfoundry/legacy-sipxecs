#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# This is a temporary script.  It is serving a purpose until this project is better
# integrated into the build.

if [ $# -lt 1 -o $# -gt 2 ]; then
  echo "Usage: ${0} <IP Address>"
  exit 1
fi

tar czf interop.tgz interop-out/

scp interop.tgz root@$1:/tmp

ssh root@$1 "rm -rf /tmp/interop-out ; tar xzf /tmp/interop.tgz -C /tmp 2>1 | grep -v \"in the future\""
