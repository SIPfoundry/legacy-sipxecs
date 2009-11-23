#!/bin/sh
echo "Initiating a call between 201 and 202 as 201 from outside "
echo "the sipx domain using HTTP post."
echo "You can call this from inside the sipx domain or outside."
echo "The password is not needed inside."
echo "use RFC 3275 flow 4. For call setup and then REFER the calling party"
curl  -k -X POST -u user1:123  https://sipxtest.sipxtest.net:6666/callcontroller/user1/user2?action=transfer\&target=user3
