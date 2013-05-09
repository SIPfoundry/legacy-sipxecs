#!/bin/sh
# dart sdk tarball perms are a little strange, repair here
find /opt/dart-sdk -perm /u+x -exec chmod ga+x {} \;
find /opt/dart-sdk -perm /u+r -exec chmod ga+r {} \;
 
