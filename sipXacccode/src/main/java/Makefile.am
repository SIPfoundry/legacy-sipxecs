include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(sipxacc_SRC)

noinst_DATA = javac-sipxacc

sipxacc_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

sipxacc_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(sipxacc_PKGS))

jardir = @SIPX_JAVADIR@/sipXacccode
JAR_FILE = sipXacccode.jar
jar_DATA = $(JAR_FILE)

$(JAR_FILE) : javac-sipxacc Manifest.txt Makefile
	jar cfm $@ Manifest.txt -C $(sipxacc_JAVAROOT) .
