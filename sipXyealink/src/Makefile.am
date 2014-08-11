include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(yealink_SRC) \
	$(yealink_RESOURCES)

noinst_DATA = javac-yealink

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXyealink.jar
jar_DATA = $(JAR_FILE)

yealink_SRC = $(shell cd $(srcdir); find org -name '*.java' -type f )
yealink_RESOURCES = sipxplugin.beans.xml

yealink_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(yealink_PKGS))

$(JAR_FILE) : javac-yealink $(yealink_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(yealink_RESOURCES))

style_SRC = $(yealink_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^

