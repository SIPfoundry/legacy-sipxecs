#!/bin/sh

# A simple script that will exit with exit code 0 and provide result if the following set of parameters are passed
if test "$1" = "-t" -a "$2" = "example.org" -a "$3" = "test1.example.org/1.2.3.4" -a "$4" = "-o" -a "$5" = "test2.example.org/4.3.2.1" -a "$#" = "9"; then
		exit 0
elif test "$1" = "example.org" -a "$2" = "test1.example.org/1.2.3.4" -a "$3" = "-o" -a "$4" = "test2.example.org/4.3.2.1" -a "$5" == "--port-TCP" -a "$6" = "5061" -a "$7" = "--port-UDP" -a "$8" = "5061" -a "$#" = "8"; then
        echo -n "DNS Records"
		exit 0
else
        echo -n "DNS Configuration ERROR"
        exit 1
fi
