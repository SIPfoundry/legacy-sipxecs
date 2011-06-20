include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(lgnortel_SRC) \
	$(lgnortel_RESOURCES)


noinst_DATA = javac-lgnortel

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXlg-nortel.jar
jar_DATA = $(JAR_FILE)

lgnortel_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
lgnortel_RESOURCES = sipxplugin.beans.xml

lgnortel_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(lgnortel_PKGS))

$(JAR_FILE) : javac-lgnortel $(lgnortel_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(lgnortel_RESOURCES))

style_SRC = $(lgnortel_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^

