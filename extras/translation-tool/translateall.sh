#!/bin/sh
export source_language=ENGLISH
export target_language=FRENCH
mkdir $target_language
for f in `cat sourcefiles`
do
	echo $f
	export bname=`basename $f`
	java -classpath "./lib/google-api-translate-java-0.91.jar:./classes"  -Dsource.file=$f -Dtarget.file=`dirname $f`/$target_language"-"$bname -Dsource.language=$source_language -Dtarget.language=$target_language org.sipfoundry.translator.Translator
done
