#!/bin/sh
echo "testA - you need a user1 with pin 123 to invoke this from outside the domain."
curl --digest -u user1:123 -k  -X POST  http://sipxtest.sipxtest.net:6667/testplugin/a/paramA?agent=user1
echo "testB -- This will fail from the local domain and remote domain." 
curl -k  -X POST  http://sipxtest.sipxtest.net:6667/testplugin/b/paramB
