include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(xtest_SRC)

xtest_SRC = \
	$(cd $(srcdir); find org -name '*.java') \
	log4j.properties \
	sipxplugin.bean.xml \
	skin.properties
