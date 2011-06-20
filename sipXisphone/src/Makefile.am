include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(isphone_SRC) \
	$(isphone_RESOURCES)

noinst_DATA = javac-isphone

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXisphone.jar
jar_DATA = $(JAR_FILE)

isphone_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
isphone_RESOURCES = sipxplugin.beans.xml

isphone_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(isphone_PKGS))

$(JAR_FILE) : javac-isphone $(isphone_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(isphone_RESOURCES))

style_SRC = $(isphone_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
