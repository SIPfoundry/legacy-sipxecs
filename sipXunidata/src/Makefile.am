include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(unidata_SRC) \
	$(unidata_RESOURCES)

noinst_DATA = javac-unidata

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXunidata.jar
jar_DATA = $(JAR_FILE)

unidata_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
unidata_RESOURCES = sipxplugin.beans.xml

unidata_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(unidata_PKGS))

$(JAR_FILE) : javac-unidata $(unidata_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(unidata_RESOURCES))

style_SRC = $(unidata_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
