## Process this file with automake to produce Makefile.in

include $(top_srcdir)/config/subdir.am

abs_top_builddir=`cd .. && pwd`

ANT_CMD=JAVA_HOME=$(JAVA_HOME) @ANT@
ANT_FLAGS=-Dtop.build.dir=$(abs_top_builddir) -f $(srcdir)/build.xml

all : all-ant

.PHONY: all-ant
all-ant :
	$(ANT_CMD) $(ANT_FLAGS)

clean :
	$(ANT_CMD) $(ANT_FLAGS) clean

.PHONY: doc
doc :
	$(ANT_CMD) $(ANT_FLAGS) doc

install :
	$(ANT_CMD) $(ANT_FLAGS) -Ddest.dir=$(DESTDIR) -Ddata.dir=$(datadir) install

check :
	$(ANT_CMD) $(ANT_FLAGS) test


EXTRA_DIST = \
   build.xml
