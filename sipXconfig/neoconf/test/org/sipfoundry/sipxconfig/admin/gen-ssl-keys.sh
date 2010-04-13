#!/bin/sh
if [ "${2}" = "invalidCA.crt" ]
then
  exit 1
else
  echo -n "VALID"
fi
