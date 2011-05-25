include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(isphone_SRC) \
	$(isphone_RESOURCES)

noinst_DATA = javac-isphone

jardir = @SIPX_JAVADIR@/sipXconfig
JAR_FILE = sipXisphone.jar
jar_DATA = $(JAR_FILE)

isphone_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
isphone_RESOURCES = sipxplugin.beans.xml

isphone_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(isphone_PKGS))

$(JAR_FILE) : javac-isphone $(isphone_RESOURCES)
	jar cf $@.tmp -C $(JAVAROOT) .
	cd $(srcdir); jar uf $(abspath $@.tmp) $(isphone_RESOURCES)
	mv $@.tmp $@
