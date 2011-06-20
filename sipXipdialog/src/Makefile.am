include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(ipdialog_SRC) \
	$(ipdialog_RESOURCES)

noinst_DATA = javac-ipdialog

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipXipdialog.jar
jar_DATA = $(JAR_FILE)

ipdialog_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
ipdialog_RESOURCES = sipxplugin.beans.xml

ipdialog_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(ipdialog_PKGS))

$(JAR_FILE) : javac-ipdialog $(ipdialog_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(ipdialog_RESOURCES))

style_SRC = $(ipdialog_SRC)
precommit : style validate-xml

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
