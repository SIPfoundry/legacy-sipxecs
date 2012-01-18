#!/bin/bash

OK="example.org test1.example.org/1.2.3.4 test2.example.org/4.3.2.1 --port-TCP 5061 --port-UDP 5061 --port-TLS 5062"
if test "$*" == "$OK" -o "$*" == "-t $OK" ; then
  echo -n "DNS Records"
  exit 0
else
  echo -n "DNS Configuration ERROR"
  exit 1
fi
