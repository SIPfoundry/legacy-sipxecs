include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(polycom_SRC) \
	$(polycom_RESOURCES)

noinst_DATA = javac-polycom

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXpolycom.jar
jar_DATA = $(JAR_FILE)

polycom_SRC = $(shell cd $(srcdir); find org -name '*.java' -type f )
polycom_RESOURCES = sipxplugin.beans.xml

polycom_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(polycom_PKGS))

$(JAR_FILE) : javac-polycom $(polycom_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(polycom_RESOURCES))

style_SRC = $(polycom_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^

