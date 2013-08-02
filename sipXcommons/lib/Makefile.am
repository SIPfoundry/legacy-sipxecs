include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

jardir = @SIPX_JAVADIR@/sipXcommons
dist_jar_DATA = \
	.java-dep

dist_jars = \
	$(shell cd ; find . -name '*.jar')

# Doing an rsync instead of a automake install will effectively uninstall jars
# that have been removed from sipxcommons.  This includes older versions of jars
# or jars that are no longer needed. We exclude sipx* because some projects
# [incorrectly] install their jars into this directory.
install-data-hook :
	rsync -av --delete \
	  --exclude='.java-dep' \
	  --exclude='Makefile*' \
	  --exclude='*.dtd' \
	  --exclude='README' \
	  --exclude='sipx*.jar' \
	  $(srcdir)/ $(jardir)/
