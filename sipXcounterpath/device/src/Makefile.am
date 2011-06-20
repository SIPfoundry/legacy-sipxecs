include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(counterpath_SRC) \
	$(counterpath_RESOURCES)

noinst_DATA = javac-counterpath

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXcounterpath.jar
jar_DATA = $(JAR_FILE)

counterpath_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
counterpath_RESOURCES = sipxplugin.beans.xml

counterpath_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(counterpath_PKGS))

$(JAR_FILE) : javac-counterpath $(counterpath_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(counterpath_RESOURCES))

style_SRC = $(counterpath_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
