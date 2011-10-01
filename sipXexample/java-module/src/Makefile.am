include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(example_SRC) \
	$(example_RESOURCES)

jardir = @SIPX_JAVADIR@/sipXconfig/plugins
JAR_FILE = sipxexample.jar
jar_DATA = $(JAR_FILE)

example_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
example_RESOURCES = sipxplugin.beans.xml

example_PKGS = \
	sipxconfig-neoconf

example_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig,$(example_PKGS))

$(JAR_FILE) : javac-example $(example_RESOURCES)
	jar cf $@ \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(example_RESOURCES))

# style checks java source code coding style agains proper sipxecs conforming. Not nec.
# but you will probably find very useful
style_SRC = $(example_SRC)

precommit : style validate-xml;

validate-xml : sipxplugin.beans.xml
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
