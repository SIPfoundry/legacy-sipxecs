include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(gtek_SRC) \
	$(gtek_RESOURCES)

noinst_DATA = javac-gtek

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXgtek.jar
jar_DATA = $(JAR_FILE)

gtek_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
gtek_RESOURCES = sipxplugin.beans.xml

gtek_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(gtek_PKGS))

$(JAR_FILE) : javac-gtek $(gtek_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(gtek_RESOURCES))

style_SRC = $(gtek_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
