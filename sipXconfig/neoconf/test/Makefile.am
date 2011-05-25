include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(srcdir)/../common.am

noinst_DATA = javac-test

test_PKGS = \
	$(neoconf_PKGS) \
	junit \
	easymock \
	easymockclassextension \
	dbunit \
	junitperf \
	p6spy \
	xmlunit \
	xercesImpl \
	xml-apis \
	spring-mock

test_DEPS = \
	$(JAVAROOT) \
	../src/classes \
	$(srcdir)/../src \
	$(srcdir) \
	$(call JavaDep, @SIPX_JAVADIR@/sipXcommons $(top_srcdir)/lib, $(test_PKGS)) \
	@SIPX_JAVADIR@/sipXconfig/agent-bindings.jar \
	@SIPX_JAVADIR@/sipXconfig/cdr-bindings.jar

test_SRC = $(shell cd $(srcdir); find org -name '*.java')
