include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

noinst_DATA = javac-callcntlr

EXTRA_DIST = \
	$(callcntlr_SRC) \
	$(callcntlr_RESOURCES)

jardir = @SIPX_JAVADIR@/sipXrest/plugins
JAR_FILE = sipXcallController.jar
jar_DATA = \
	$(JAR_FILE)

callcntlr_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

callcntlr_RESOURCES = \
	plugin.xml

callcntlr_DEPS = \
	$(JAVAROOT) \
	@SIPX_JAVADIR@/sipXrest/sipxrest.jar \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(callcntlr_PKGS))

$(JAR_FILE) : javac-callcntlr Manifest.txt $(callcntlr_RESOURCES)
	jar cfm $@ Manifest.txt \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(callcntlr_RESOURCES))
