#!/bin/sh
echo "Initiating a call between 201 and 202 as 201 from the sipx domain using HTTP post"
curl  -k -X POST   https://sipxtest.sipxtest.net:6666/callcontroller/201/202
sleep 20
echo "Query call setup progress for call between 201 and 202 as 201"
curl  -k -u user1:123 https://sipxtest.sipxtest.net:6666/callcontroller/201/202
