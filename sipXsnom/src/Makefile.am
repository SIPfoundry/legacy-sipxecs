include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(snom_SRC) \
	$(snom_RESOURCES)

noinst_DATA = javac-snom

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXsnom.jar
jar_DATA = $(JAR_FILE)

snom_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
snom_RESOURCES = sipxplugin.beans.xml

snom_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(snom_PKGS))

$(JAR_FILE) : javac-snom $(snom_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(snom_RESOURCES))

style_SRC = $(snom_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^

