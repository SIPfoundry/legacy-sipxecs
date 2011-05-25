include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/src/common.am

SUBDIRS = \
	java
	.

jardir = @SIPX_JAVADIR@/sipXacccode
JAR_FILE = sipXacccode.jar
jar_DATA = $(JAR_FILE)

$(JAR_FILE) : Makefile Manifest.txt
	jar -cfm $@ Manifest.txt -C $(sipxacc_JAVAROOT) .
