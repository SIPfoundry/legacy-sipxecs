#!/bin/sh
export COMMONS=/usr/local/sipx/share/java/sipXecs/sipXcommons/
classpath="symclient.jar:$COMMONS/log4j.jar:$COMMONS/dnsjava.jar:$COMMONS/commons-collections.jar:$COMMONS/commons-digester.jar:$COMMONS/commons-beanutils.jar:$COMMONS/commons-logging.jar:$COMMONS/commons-logging-api.jar:$COMMONS/xmlrpc-common.jar:$COMMONS/xmlrpc-server.jar:$COMMONS/xmlrpc-client.jar:$COMMONS/ws-commons-util.jar:$COMMONS/junit.jar:tools/sipunit.jar:lib/Stun4J.jar:$COMMONS/sipxcommons.jar:./classes"

javac -classpath $classpath -sourcepath src/main/java -d classes -g:source,lines,vars src/test/java/org/sipfoundry/sipxrelay/*.java
jar -cvf symclient.jar ./classes 

java -classpath "$classpath" -Dconf.dir=./  -Dtester.address=192.168.5.240 -Dtester.callLoad=1 -Dtester.npackets=1000 -Dtester.startport=30000 org.sipfoundry.sipxrelay.SymmitronThruputTest

