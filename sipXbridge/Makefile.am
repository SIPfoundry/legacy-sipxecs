## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/project.am
include ant-targets.am

editscripts_IN = \
    sipxbridge.sh.in

editscripts :
	@for f in $(editscripts_IN); \
	do \
	  target=`basename $$f .in`; \
     $(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	  $(INSTALL) -D -m 755 $$target $(DESTDIR)$(bindir)/$$target; \
	done


install-exec-hook : editscripts

RPMS=$(RPM)

.PHONY : editscripts
EXTRA_DIST = \
  README \
  config/sipX-config.in \
  config/sipX-buildstamp.cpp.in \
  config/sipX-buildstamp.h.in \
  config/svn-version \
  ant-targets.xml \
  build.xml \
  CONTRIBUTORS \
  BUILDSTAMP \
  SVN-VERSION \
  LICENSE \
  config.properties \
  ant-targets.xml \
  sipxbridge.spec \
  src \
  lib

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties

config.properties : config.properties.in Makefile
	$(LocalizeSipXconfig) $(srcdir)/config.properties.in > config.properties

