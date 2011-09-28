include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am
include $(top_srcdir)/src/common.am

noinst_DATA = javac-test

EXTRA_DIST = \
	$(test_SRC) \
	$(test_RESOURCES)

test_SRC = $(shell cd $(srcdir); find . -name '*.java')
test_RESOURCES = $(shell cd $(srcdir); find -type f -not -name '*.java')

test_DEPS = \
	$(JAVAROOT) \
	$(commons_classes) \
	$(srcdir) \
	$(call JavaDep, $(top_srcdir)/lib, \
	  junit \
	  jibx-run \
	  $(commons_PKGS) \
	)

test_ENV = \
	LD_LIBRARY_PATH=../../main/native/.libs
