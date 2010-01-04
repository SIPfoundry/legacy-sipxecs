#!/bin/sh
echo "java -cp \"./lib/google-api-translate-java-0.91.jar:./classes\"  $1 $2 $3 $4 org.sipfoundry.translator.Translator"
java -classpath "./lib/google-api-translate-java-0.91.jar:./classes"  $1 $2 $3 $4 org.sipfoundry.translator.Translator

