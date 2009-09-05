#!/bin/sh
echo "testA - you need a user1 with pin 123 to invoke this from outside the domain."
curl --digest -u user1:123 -k  -X POST  https://sipxtest.sipxtest.net:6666/testplugin/a/paramA?agent=user1
echo "testB -- you can only run this on the local domain. This should fail from outside the sipx proxy domain."
curl -k  -X POST  https://sipxtest.sipxtest.net:6666/testplugin/b/paramB
