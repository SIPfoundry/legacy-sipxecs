#!/bin/bash
#
# Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
# Contributors retain copyright to elements licensed under a Contributor Agreement.
# Licensed to the User under the LGPL license.
#
###################################################

# This is a temporary script.  It is serving a purpose until this project is better
# integrated into the build.

rm -rf interop-out
mkdir interop-out

SIPXCONFIG_JAR=../../INSTALL/share/java/sipXecs/sipXconfig/sipxconfig.jar
javac src/GetSpecialIds.java -cp $SIPXCONFIG_JAR
if [ $? != 0 ]; then
   echo "ERROR: Failed to compile src/GetSpecialIds.java!" >&2
   exit 1
fi
INTEROP_SPECIAL_IDS=`java -classpath "$SIPXCONFIG_JAR:./src" GetSpecialIds`
if [ $? != 0 ]; then
   echo "ERROR: Failed to run GetSpecialIds" >&2
   exit 2
fi
rm src/GetSpecialIds.class

SVN_PATH="."
INTEROP_SVN_URL=`svn info $SVN_PATH | grep "^URL:" | ruby -e 'puts STDIN.readline.gsub("\/", "\\\\/").gsub("https", "http")'`
INTEROP_SVN_REVISION=`svn info $SVN_PATH | grep "^Revision:"`
INTEROP_SVN_LAST_CHANGE=`svn info $SVN_PATH | grep "^Last Changed Date:"`

function process_substitutions { # file
   sed -i \
      -e 's/@SIPX_DATADIR@/\/usr\/share\/sipxecs/g' \
      -e 's/@SERVICEDIR@/\/etc\/init.d/g' \
      -e 's/@SIPX_DBDIR@/\/var\/sipxdata\/sipdb/g' \
      -e 's/@SIPX_CONFDIR@/\/etc\/sipxpbx/g' \
      -e 's/@SIPX_LOGDIR@/\/var\/log\/sipxpbx/g' \
      -e 's/@HTTP_ROOTDIR@/\/usr\/share\/www\/doc/g' \
      -e 's/@SIPXPBXUSER@/sipxchange/g' \
      -e 's/@SIPXPBXGROUP@/sipxchange/g' \
      -e 's/@INTEROP_SPECIAL_IDS@/'"$INTEROP_SPECIAL_IDS"'/g' \
      -e 's/@INTEROP_DEFAULT_SERIAL_EXPIRES@/5/g' \
      -e 's/@INTEROP_SVN_URL@/'"$INTEROP_SVN_URL"'/g' \
      -e 's/@INTEROP_SVN_REVISION@/'"$INTEROP_SVN_REVISION"'/g' \
      -e 's/@INTEROP_SVN_LAST_CHANGE@/'"$INTEROP_SVN_LAST_CHANGE"'/g' \
      $1
}

# Process install/revert scripts
cp -f bin/sipxinterop-setup.in.sh interop-out/sipxinterop-setup
chmod +x interop-out/sipxinterop-setup
process_substitutions interop-out/sipxinterop-setup
cp -f bin/sipxinterop-revert.in.sh interop-out/sipxinterop-revert
chmod +x interop-out/sipxinterop-revert
process_substitutions interop-out/sipxinterop-revert

# Copy 'parts' content.
mkdir interop-out/parts
cp parts/* interop-out/parts

# Process HTTP content
mkdir interop-out/http_rootdir
HTTPD_ROOTDIR_FILES=`ls http_rootdir`
for filename in $HTTPD_ROOTDIR_FILES; do
   new_filename=`ruby -e 'puts ARGV[0].sub(".in", "").sub(".pl", "").sub(".sh", "")' $filename`
   cp -f http_rootdir/$filename interop-out/http_rootdir/$new_filename
   process_substitutions interop-out/http_rootdir/$new_filename
done



