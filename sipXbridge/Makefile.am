## Process this file with automake to produce Makefile.in
include $(top_srcdir)/config/project.am
include ant-targets.am

bin_SCRIPTS = sipxbridge.sh


RPMS=$(RPM)

EXTRA_DIST = \
  README \
  config/sipX-config.in \
  config/sipX-buildstamp.cpp.in \
  config/sipX-buildstamp.h.in \
  config/svn-version \
  ant-targets.xml \
  build.xml \
  config.properties.in \
  sipxbridge.spec.in \
  sipxbridge.spec \
  sipxbridge.process.xml.in \
  sipxbridge.sh.in \
  CONTRIBUTORS \
  BUILDSTAMP \
  SVN-VERSION \
  LICENSE \
  ant-targets.xml \
  src \
  lib

# BUILT_SOURCES is special autoconf variable to be checked on every make
BUILT_SOURCES = config.properties


ETC_FILES = \
    sipxbridge.process.xml.in

install-data-hook : 
	@for f in $(ETC_FILES); \
	do \
	  	target=`basename $$f .in`; \
		echo "Processing  $(srcdir)/$$f"; \
      		echo "Localize $$f -> $$target"; \
		$(LocalizeSipXconfig) $(srcdir)/$$f > $$target; \
	    	mkdir -p $(DESTDIR)$(SIPX_PROCDIR); \
	    	echo "Installing default $(DESTDIR)$(SIPX_PROCDIR)/$$target"; \
		$(INSTALL) -D -m 644 $$target $(DESTDIR)$(SIPX_PROCDIR)/$$target; \
	done

config.properties : config.properties.in 
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxbridge.sh : sipxbridge.sh.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@

sipxbridge.spec : sipxbridge.spec.in
	@echo "Localize $< -> $@"
	@$(LocalizeSipXconfig) $< > $@
