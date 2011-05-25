include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/../common.am

EXTRA_DIST = \
	$(neoconf_SRC)

noinst_DATA = javac-neoconf

JAR_FILE = sipxconfig.jar

jardir = @SIPX_JAVADIR@/sipXconfig
jar_DATA = $(JAR_FILE)

neoconf_RESOURCES = \
	$(shell cd $(srcdir); find . -type f \( \
	  -name '*.json' \
	  -o -name '*.properties' \
	  -o -name '*.xml' \
	  -o -name '*.xsd' \
	\))

$(JAR_FILE) : javac-neoconf Manifest.txt
	jar cfm $@.tmp Manifest.txt -C $(JAVAROOT) .
	cd $(srcdir); jar uf $(abspath $@.tmp) $(neoconf_RESOURCES)
	mv $@.tmp $@

neoconf_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(neoconf_PKGS)) \
	@SIPX_JAVADIR@/sipXconfig/agent-bindings.jar \
	@SIPX_JAVADIR@/sipXconfig/cdr-bindings.jar

neoconf_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')
