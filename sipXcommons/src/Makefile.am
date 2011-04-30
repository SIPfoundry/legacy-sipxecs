include $(top_srcdir)/config/utility.am
SUBDIRS = \
	main \
	test \
	.

jardir = @SIPX_JAVADIR@/sipXcommons
jar_DATA = \
	$(jar_deps)

jar_deps = $(shell find $(top_srcdir)/lib -name '*.jar')
