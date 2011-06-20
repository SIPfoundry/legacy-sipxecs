include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(rest_SRC)

noinst_DATA = javac-rest

jardir = @SIPX_JAVADIR@/sipXrest
JAR_FILE = sipxrest.jar
jar_DATA = \
	$(JAR_FILE)

rest_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

rest_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(rest_PKGS))

$(JAR_FILE) : javac-rest Manifest.txt
	jar -cfm $@ Manifest.txt -C $(JAVAROOT) .
