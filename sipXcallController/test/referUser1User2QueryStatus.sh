#!/bin/sh
echo "Initiating a call between 201 and 202 as 201 from outside the sipx domain using HTTP post"
echo "You can call this from inside the sipx domain or outside. The password is not needed inside."
echo "curl  -k -X POST -u user1:123  https://sipxtest.sipxtest.net:6666/callcontroller/user1/user2?timeout=10\&resultCacheTime=30"
curl  -k -X POST -u user1:123  https://sipxtest.sipxtest.net:6666/callcontroller/user1/user2?timeout=10\&resultCacheTime=30
sleep 20
echo "Query call setup progress for call between 201 and 202 as 201"
curl -k -u user1:123 https://sipxtest.sipxtest.net:6666/callcontroller/user1/user2
echo "After 30 seconds run query.sh"
