include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/common.am

EXTRA_DIST = \
	$(unidata_SRC) \
	$(unidata_RESOURCES)

noinst_DATA = javac-unidata

jardir = @SIPX_JAVADIR@/sipXconfig
JAR_FILE = sipXunidata.jar
jar_DATA = $(JAR_FILE)

unidata_SRC = $(shell cd $(srcdir); find org -type f -name '*.java')
unidata_RESOURCES = sipxplugin.beans.xml

unidata_DEPS = \
	$(call JavaDep,@SIPX_JAVADIR@/sipXconfig @SIPX_JAVADIR@/sipXcommons,$(unidata_PKGS))

$(JAR_FILE) : javac-unidata $(unidata_RESOURCES)
	jar cf $@.tmp -C $(JAVAROOT) .
	cd $(srcdir); jar uf $(abspath $@.tmp) $(unidata_RESOURCES)
	mv $@.tmp $@
