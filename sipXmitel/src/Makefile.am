include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(mitel_SRC) \
	$(mitel_RESOURCES)

noinst_DATA = javac-mitel

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXmitel.jar
jar_DATA = $(JAR_FILE)

mitel_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
mitel_RESOURCES = sipxplugin.beans.xml

mitel_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(mitel_PKGS))

$(JAR_FILE) : javac-mitel $(mitel_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(mitel_RESOURCES))

style_SRC = $(mitel_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^

