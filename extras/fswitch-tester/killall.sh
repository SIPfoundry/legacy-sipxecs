#!/bin/sh
for f in `ps auxw | grep FreeSwitchTester | cut  -d ' ' -f 5`
do
	echo "PID  = " $f
	kill -1 $f
done

