include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/device/src/common.am

EXTRA_DIST = \
	$(test_SRC)

noinst_DATA = javac-test

test_PKGS = \
	$(counterpath_PKGS) \
	sipxconfig-test

test_DEPS = \
	$(JAVAROOT) \
	../src/classes \
	$(srcdir)/../src \
	$(srcdir) \
	$(call JavaDep,@SIPX_JAVADIR@/sipXcommons @SIPX_JAVADIR@/sipXconfig, $(test_PKGS))

test_JAVAFLAGS = -Dclasspath.dir=$(srcdir) -Dbasedir=$(top_builddir)

test_SRC = $(shell cd $(srcdir); find org -name '*.java')

javac-test : test.properties

local.etc.dir = $(abspath $(top_srcdir)/etc)
test.properties : Makefile
	@echo -e "$(subst $(space),\n,$(foreach V,local.etc.dir $(CommonVariablesNames),$(V)=$($(V))))" > $@

precommit : check

