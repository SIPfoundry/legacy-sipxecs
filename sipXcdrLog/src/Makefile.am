include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

noinst_DATA = javac-cdrlog

jardir = @SIPX_JAVADIR@/sipXrest/plugins
JAR_FILE = sipxcdrLog.jar
jar_DATA = \
	$(JAR_FILE)

EXTRA_DIST = \
	$(cdrlog_SRC) \
	$(cdrlog_RESOURCES)

cdrlog_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

cdrlog_RESOURCES = \
	plugin.xml

cdrlog_DEPS = \
	$(JAVAROOT) \
	@SIPX_JAVADIR@/sipXrest/sipxrest.jar \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(cdrlog_PKGS))

$(JAR_FILE) : javac-cdrlog Manifest.txt $(cdrlog_RESOURCES)
	jar cfm $@ Manifest.txt \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(cdrlog_RESOURCES))