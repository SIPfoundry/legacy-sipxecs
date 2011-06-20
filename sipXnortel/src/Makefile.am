include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(nortel_SRC) \
	$(nortel_RESOURCES)

noinst_DATA = javac-nortel

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXnortel.jar
jar_DATA = $(JAR_FILE)

nortel_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
nortel_RESOURCES = sipxplugin.beans.xml

nortel_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(nortel_PKGS))

$(JAR_FILE) : javac-nortel $(nortel_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(nortel_RESOURCES))

style_SRC = $(nortel_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^

