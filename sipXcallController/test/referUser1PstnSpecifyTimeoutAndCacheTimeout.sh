echo "this should fail if you run it from a remote location a remote location. Will only work from a trusted agent"
echo "Initiate a call to another cell phone. tiemout in 10 sec "
curl -k  -X POST  https://sipxtest.sipxtest.net:6666/callcontroller/user1/933013402561?agent=user2\&timeout=10\&cachetimeout=30
sleep 10
curl -k  -X GET  https://sipxtest.sipxtest.net:6666/callcontroller/user1/933013402561?agent=user2
