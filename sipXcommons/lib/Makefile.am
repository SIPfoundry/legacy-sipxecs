include $(top_srcdir)/config/utility.am
include $(top_srcdir)/config/java.am

EXTRA_DIST = \
	$(shell cd $(srcdir); find . -name '*.jar')

jardir = @SIPX_JAVADIR@/sipXcommons
dist_jar_DATA = \
	.java-dep

# Doing an rsync instead of a automake install will effectively uninstall jars
# that have been removed from sipxcommons.  This includes older versions of jars
# or jars that are no longer needed. We exclude sipx* because some projects
# [incorrectly] install their jars into this directory.
install-data-hook :
	$(INSTALL) -d -m 755 $(DESTDIR)$(jardir)
	rsync -av --delete \
	  --exclude='.java-dep' \
	  --exclude='Makefile*' \
	  --exclude='*.dtd' \
	  --exclude='README' \
	  --exclude='sipx*.jar' \
	  $(srcdir)/ $(DESTDIR)$(jardir)/
