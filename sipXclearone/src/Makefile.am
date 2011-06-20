include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(clearone_SRC) \
	$(clearone_RESOURCES)

noinst_DATA = javac-clearone

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXclearone.jar
jar_DATA = $(JAR_FILE)

clearone_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
clearone_RESOURCES = sipxplugin.beans.xml

clearone_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(clearone_PKGS))

$(JAR_FILE) : javac-clearone $(clearone_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(clearone_RESOURCES))

style_SRC = $(clearone_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
