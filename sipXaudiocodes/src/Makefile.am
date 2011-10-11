include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(audiocodes_SRC) \
	$(audiocodes_RESOURCES)

noinst_DATA = javac-audiocodes

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXaudiocodes.jar
jar_DATA = $(JAR_FILE)

audiocodes_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
audiocodes_RESOURCES = sipxplugin.beans.xml

audiocodes_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(audiocodes_PKGS))

$(JAR_FILE) : javac-audiocodes $(audiocodes_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(audiocodes_RESOURCES))

style_SRC = $(audiocodes_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
