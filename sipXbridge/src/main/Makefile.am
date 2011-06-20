include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

SUBDIRS = \
	java \
	.

jardir = @SIPX_JAVADIR@/sipXbridge
JAR_FILE = sipXbridge.jar
jar_DATA = \
	$(JAR_FILE)

$(JAR_FILE) : Manifest.txt
	jar -cfm $@ Manifest.txt -C java/classes .
