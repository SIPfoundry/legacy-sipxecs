include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(aastra_SRC) \
	$(aastra_RESOURCES)

noinst_DATA = javac-aastra $(JAR_FILE)

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXaastra.jar
jar_DATA = $(JAR_FILE)

aastra_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
aastra_RESOURCES = sipxplugin.beans.xml

aastra_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(aastra_PKGS))

$(JAR_FILE) : javac-aastra $(aastra_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(aastra_RESOURCES))

style_SRC = $(aastra_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^

