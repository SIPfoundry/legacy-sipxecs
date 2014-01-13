#!/bin/sh

case "$1" in
  --list)
cat <<EOF
201401010000 configuration.tar.gz
201401020000 configuration.tar.gz voicemail.tar.gz
201401030000
EOF
  ;;
  --backup)
echo "mock configuration backup" > configuration.tar.gz
echo "mock voicemail backup" > voicemail.tar.gz
echo "mock cdr backup" > cdr.tar.gz
echo "mock device_config backup" > device_config.tar.gz
  ;;
esac
