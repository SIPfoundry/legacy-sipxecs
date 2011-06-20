include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(cisco_SRC) \
	$(cisco_RESOURCES)

noinst_DATA = javac-cisco $(JAR_FILE)

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXcisco.jar
jar_DATA = $(JAR_FILE)

cisco_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
cisco_RESOURCES = sipxplugin.beans.xml

cisco_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(cisco_PKGS))

$(JAR_FILE) : javac-cisco $(cisco_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(cisco_RESOURCES))

style_SRC = $(cisco_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
