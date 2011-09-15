include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/common.am

EXTRA_DIST = \
	$(sipxacc_SRC) \
	$(sipxacc_RESOURCES)

noinst_DATA = javac-sipxacc

sipxacc_SRC = \
	$(shell cd $(srcdir); find org -type f -name '*.java')
sipxacc_RESOURCES = org/sipfoundry/authcode/AuthCode.properties

sipxacc_DEPS = \
	$(JAVAROOT) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons, $(sipxacc_PKGS))

jardir = @SIPX_JAVADIR@/sipXacccode
JAR_FILE = sipXacccode.jar
jar_DATA = $(JAR_FILE)

$(JAR_FILE) : javac-sipxacc $(sipxacc_RESOURCES) Manifest.txt Makefile
	jar cfm $@ Manifest.txt \
	  $(call JarInclude,$(sipxacc_JAVAROOT),.) \
	  $(call JarInclude,$(srcdir),$(sipxacc_RESOURCES))
