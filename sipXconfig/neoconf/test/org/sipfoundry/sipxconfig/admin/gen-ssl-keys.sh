#!/bin/sh
if [ "${2}" = "validCA.crt" ]
then
  echo -n "VALID"
else
  exit 1
fi