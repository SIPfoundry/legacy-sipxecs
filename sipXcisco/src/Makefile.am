include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(cisco_SRC) \
	$(cisco_RESOURCES)

noinst_DATA = javac-cisco $(JAR_FILE)

jardir = @SIPX_JAVADIR@/sipXconfig
JAR_FILE = sipXcisco.jar
jar_DATA = $(JAR_FILE)

cisco_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
cisco_RESOURCES = sipxplugin.beans.xml

cisco_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(cisco_PKGS))

$(JAR_FILE) : javac-cisco $(cisco_RESOURCES)
	jar cf $@.tmp -C $(JAVAROOT) .
	cd $(srcdir); jar uf $(abspath $@.tmp) $(cisco_RESOURCES)
	mv $@.tmp $@
