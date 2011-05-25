include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(jar_SRC)

noinst_DATA = javac-recording

jardir = @SIPX_JAVADIR@/sipXrecording/sipXrecording.jar
jar_DATA = $(JAR_FILE)

recording_PKGS = \
	sipxcommons \
	log4j \
	commons-io \
	commons-codec \
	commons-httpclient \
	commons-logging \
	javax.servlet \
	jetty

recording_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')

recording_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(recording_PKGS))

$(JAR_FILE) : javac-recording
	jar -cf $@ -C $(JAVAROOT) .
