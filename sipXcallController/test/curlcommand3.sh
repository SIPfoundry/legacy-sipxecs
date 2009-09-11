echo "this should fail if you run it from a remote location a remote location. Will only work from a trusted agent"
echo "Initiate a call from user 1 to user 3 by user2 acting as an agent"
curl -u user1:123 -k  -X POST  https://sipxtest.sipxtest.net:6666/callcontroller/user1/922404622570?agent=user2
