#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

echo "Content-Type: text/plain"
echo ""

echo -n "Configuration at "
date
echo ""

echo "Server version:"
echo -n "  " 
uname -a
echo ""

echo "Interop Configuration version:"
echo "  @INTEROP_SVN_URL@"
echo "  @INTEROP_SVN_REVISION@"
echo "  @INTEROP_SVN_LAST_CHANGE@"
echo ""

echo "sipXecs versions:"
rpm -qa | grep sipx | sort | ruby -e 'STDIN.readlines.each {|line| puts "  #{line}"}'
echo ""




