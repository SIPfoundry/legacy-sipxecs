include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(synchserver_SRC)

noinst_DATA = javac-synchserver

jardir = @SIPX_JAVADIR@/sipXopenfire/lib

jar_DATA = \
	$(synchserver_JAR)

synchserver_JAR = sipx-openfire-vcard-synchserver.jar
synchserver_SRC = $(shell cd $(srcdir); find src -name '*.java')
synchserver_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons,$(synchserver_PKGS)) \
	@OPENFIRE_HOME@/lib/openfire.jar

$(synchserver_JAR) : javac-synchserver Manifest.txt Makefile
	jar cfm $@ Manifest.txt \
	  $(call JarInclude,$(vcardsynchserver_JAVAROOT),.)
