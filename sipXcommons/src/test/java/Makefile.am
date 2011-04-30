java_modules = test

include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(test_SRC) \
	$(test_RESOURCES)

test_SRC = $(shell cd $(srcdir); find . -name '*.java')
test_RESOURCES = $(shell find $(srcdir) -type f -not -name '*.java')

test_DEPS = \
	$(JAVAROOT) \
	$(shell find $(top_srcdir)/lib -name '*.jar') \
	$(top_builddir)/src/main/java/classes

test_ENV = \
	LD_LIBRARY_PATH=../../main/native/.libs
