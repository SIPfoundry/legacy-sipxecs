#!/bin/sh
export COMMONS=/usr/share/java/sipXecs/sipXcommons/
classpath="$COMMONS/log4j.jar:$COMMONS/jain-sip-sdp.jar:$COMMONS/dnsjava.jar:$COMMONS/commons-collections.jar:$COMMONS/commons-digester.jar:$COMMONS/commons-beanutils.jar:$COMMONS/commons-logging.jar:$COMMONS/commons-logging-api.jar:$COMMONS/xmlrpc-common.jar:$COMMONS/xmlrpc-server.jar:$COMMONS/xmlrpc-client.jar:$COMMONS/ws-commons-util.jar:$COMMONS/junit.jar:tools/sipunit.jar:lib/Stun4J.jar:$COMMONS/sipxcommons.jar"

javac -classpath $classpath -sourcepath src/main/java -d classes -g:source,lines,vars src/test/java/org/sipfoundry/sipxbridge/symmitron/*.java

java -classpath "$classpath:./classes" -Dconf.dir=/etc/sipxpbx  -Dtester.address=192.168.5.240 -Dtester.callLoad=50 -Dtester.npackets=2000 org.sipfoundry.sipxbridge.symmitron.SymmitronThruputTest

