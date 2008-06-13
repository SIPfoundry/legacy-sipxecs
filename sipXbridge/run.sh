#!/bin/sh
rm -f /usr/local/sipx/var/log/sipxpbx/*.log
classpath="lib/log4j-1.2.14.jar:lib/jain-sip-sdp-1.2.70.jar:lib/dnsjava-2.0.6.jar:lib/Stun4J.jar:lib/commons-collections-3.2.jar:lib/commons-digester-1.8.jar:lib/commons-beanutils.jar:lib/commons-logging-1.1.1.jar:lib/commons-logging-api-1.1.1.jar:lib/xmlrpc-common-3.1.jar:lib/xmlrpc-server-3.1.jar:lib/xmlrpc-client-3.1.jar:lib/ws-commons-util-1.0.2.jar"

javac -classpath $classpath -sourcepath src/main/java -d classes -g:source,lines,vars src/main/java/org/sipfoundry/sipxbridge/*.java

java -classpath "$classpath:./classes" -Dconf.dir=/usr/local/etc/sipxpbx org.sipfoundry.sipxbridge.Gateway

