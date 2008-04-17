#!/bin/sh
rm -f /var/log/sipxpbx/sipxbridge.log
classpath="lib/log4j-1.2.14.jar:lib/jain-sip-api-1.2.jar:lib/jain-sip-ri-1.2.jar:lib/nist-sdp-1.0.jar:lib/dnsjava-2.0.6.jar:lib/Stun4J.jar:lib/commons-collections-3.2.jar:lib/commons-digester-1.8.jar:lib/commons-beanutils.jar:lib/commons-logging-1.1.1.jar:lib/commons-logging-api-1.1.1.jar:lib/xmlrpc-common-3.1.jar:lib/xmlrpc-server-3.1.jar:lib/xmlrpc-client-3.1.jar:lib/ws-commons-util-1.0.2.jar"

javac -classpath $classpath -sourcepath src/main/java -d classes -g:source,lines,vars src/main/java/org/sipfoundry/sipxbridge/*.java

java -classpath "$classpath:./classes" -Djavax.net.ssl.trustStore=keystore/testkeys -Djavax.net.ssl.keyStore=keystore/testkeys -Djavax.net.ssl.keyStorePassword=passphrase org.sipfoundry.sipxbridge.Gateway

