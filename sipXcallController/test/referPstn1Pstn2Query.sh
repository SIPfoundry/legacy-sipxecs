echo "this should fail if you run it from a remote location a remote location. Will only work from a trusted agent"
echo "Initiate a call from user 1 to user 3 by user2 acting as an agent"
curl -k  -X POST  https://sipxtest.sipxtest.net:6666/callcontroller/932404622570/933013402561?agent=user2
sleep 10
curl -k  -X GET  https://sipxtest.sipxtest.net:6666/callcontroller/932404622570/933013402561?agent=user2
