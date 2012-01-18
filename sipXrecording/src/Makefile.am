include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(recording_SRC)

noinst_DATA = javac-recording

jardir = @SIPX_JAVADIR@/sipXrecording
JAR_FILE = sipXrecording.jar
jar_DATA = $(JAR_FILE)

recording_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')

recording_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(recording_PKGS))

$(JAR_FILE) : javac-recording
	jar -cf $@ -C $(JAVAROOT) .
