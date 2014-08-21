include config/utility.am
include config/project.am

SUBDIRS = \
	. \
	applications \
	bin \
	etc

EXTRA_DIST += $(srcdir)/applications/sems/*

dist-hook:
	rm $(top_distdir)/applications/sems/pkg/rpm/sems.spec

all install:
	make -C applications/sems/build $@
