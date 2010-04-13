#!/bin/sh
if [ "${2}" = "invalidCA.crt" ]
then
  exit 1
fi

if [ "${1}" = "invalidCRT.crt" ]
then
  exit 1
fi
