#!/bin/sh
echo "Initiating a call between 201 and 202 as 201 from outside the sipx domain using HTTP post"
echo "You can call this from inside the sipx domain or outside. The password is not needed inside."
echo "The following example uses INVITE (not REFER) for call setup and hence places the call controller in the signaling path"
echo "use RFC 3275 flow 4."
curl  -k -X POST -u user1:123  https://sipxtest.sipxtest.net:6666/callcontroller/user1/user2?sipMethod=INVITE
