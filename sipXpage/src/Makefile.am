include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(page_SRC)

noinst_DATA = javac-page

jardir = @SIPX_JAVADIR@/sipXpage
JAR_FILE = sipXpage.jar
jar_DATA = $(JAR_FILE)

page_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')

page_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(page_PKGS))

$(JAR_FILE) : javac-page
	jar -cf $@ -C $(JAVAROOT) .
