include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/config/sipxconfig.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(neoconf_SRC) \
	$(neoconf_RESOURCES)

noinst_DATA = javac-neoconf

JAR_FILE = sipxconfig-neoconf.jar

jardir = @SIPX_JAVADIR@/sipXconfig
jar_DATA = $(JAR_FILE)

neoconf_RESOURCES = \
	$(shell cd $(srcdir); find . -type f \( \
	  -name '*.json' \
	  -o -name '*.properties' \
	  -o -name '*.crt' \
	  -o -name '*.xml' \
	  -o -name '*.jpg' \
	  -o -name '*.xsd' \
	\))

$(JAR_FILE) : javac-neoconf Manifest.txt $(neoconf_RESOURCES)
	jar cfm $@ Manifest.txt \
	  $(call JarInclude,$(JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(neoconf_RESOURCES))

neoconf_COMPILE_PKGS = \
	$(neoconf_PKGS) \
	junit \
	xmlunit \
	dbunit \
	easymock \
	easymockclassextension \
	spring-mock

neoconf_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(neoconf_COMPILE_PKGS)) \
	@SIPX_JAVADIR@/sipXconfig/agent-bindings.jar \
	@SIPX_JAVADIR@/sipXconfig/cdr-bindings.jar

neoconf_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')

style_DEPS = $(neoconf_DEPS)
style_SRC = $(filter-out org/taha/%,$(neoconf_SRC))
precommit : style validate-xml

SPRING_XML = \
	$(shell cd $(srcdir); find -name '*.beans.xml') \
	beanRefContext.xml

HIBERNATE_XML = \
	$(shell cd $(srcdir); find -name '*.hbm.xml')

validate-xml : validate-spring-xml;

validate-spring-xml : $(SPRING_XML)
	xmllint --noout \
	  --schema $(SIPX_DATADIR)/schema/spring-beans-2.0.xsd  \
	$^
