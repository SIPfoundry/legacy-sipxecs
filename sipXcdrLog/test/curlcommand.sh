#!/bin/sh
echo "Querying CDR Call Logs for User 221"
curl --digest -X GET -u 221:124 http://sipxtest.sipxtest.net:6667/cdr/221
sleep 20
echo "Query CDR Call Logs for User 221 from 20090828 with a limit of 5 records"
curl  --digest -X GET -u 221:124 GET http://sipxtest.sipxtest.net:6667/cdr/221?fromdate=20090828&limit=5
