include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(provision_SRC)

noinst_DATA = javac-provision

jardir = @SIPX_JAVADIR@/sipXprovision
JAR_FILE = sipXprovision.jar
jar_DATA = $(JAR_FILE)

provision_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')

provision_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(provision_PKGS))

$(JAR_FILE) : javac-provision
	jar -cf $@ -C $(JAVAROOT) .
