include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(xtest_SRC)

xtest_RESOURCES = \
	log4j.properties \
	sipxplugin.bean.xml \
	skin.properties

xtest_SRC = \
	$(shell cd $(srcdir); find org -name '*.java')
