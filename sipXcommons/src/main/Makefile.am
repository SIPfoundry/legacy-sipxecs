include $(top_srcdir)/config/utility.am

SUBDIRS = \
	java \
	native \
	.

jardir = @SIPX_JAVADIR@/sipXcommons
JAR_FILE = sipxcommons.jar
jar_DATA = \
	$(JAR_FILE)

.PHONY : $(JAR_FILE)
$(JAR_FILE) :
	jar -cf $@ -C java/classes .
