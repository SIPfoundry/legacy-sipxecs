include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(grandstream_SRC) \
	$(grandstream_RESOURCES)

noinst_DATA = javac-grandstream $(JAR_FILE)

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXgrandstream.jar
jar_DATA = $(JAR_FILE)

grandstream_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
grandstream_RESOURCES = sipxplugin.beans.xml

grandstream_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(grandstream_PKGS))

$(JAR_FILE) : javac-grandstream $(grandstream_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(grandstream_RESOURCES))

style_SRC = $(grandstream_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
