#!/bin/sh

# A simple script that will exit with exit code 0 if the -s flag 
# is passed, or exit code 1 otherwise
if [ "$1" = "-s" ]; then
	exit 0
else
        echo "called without -s flag"
	exit 1
fi
